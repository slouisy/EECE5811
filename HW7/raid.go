package main

import (
	"crypto/rand"
	"errors"
	"fmt"
	"os"
	"sync"
	"time"
)

const (
	NumDisks  = 5
	BlockSize = 4096 // 4KB
)

type RAID interface {
	Write(blockNum int, data []byte) error
	Read(blockNum int) ([]byte, error)
}

type Disk struct {
	file *os.File
	mu   sync.Mutex
}

func NewDisk(filename string) (*Disk, error) {
	f, err := os.OpenFile(filename, os.O_RDWR|os.O_CREATE, 0666)
	if err != nil {
		return nil, err
	}
	return &Disk{file: f}, nil
}

func (d *Disk) WriteBlock(blockNum int, data []byte) error {
	if len(data) != BlockSize {
		return fmt.Errorf("data must be exactly %d bytes", BlockSize)
	}
	d.mu.Lock()
	defer d.mu.Unlock()
	offset := int64(blockNum) * BlockSize
	_, err := d.file.WriteAt(data, offset)
	if err != nil {
		return err
	}
	return d.file.Sync() // ensure fsync
}

func (d *Disk) ReadBlock(blockNum int) ([]byte, error) {
	buf := make([]byte, BlockSize)
	d.mu.Lock()
	defer d.mu.Unlock()
	offset := int64(blockNum) * BlockSize
	_, err := d.file.ReadAt(buf, offset)
	if err != nil {
		return nil, err
	}
	return buf, nil
}

// RAID 0 (striping, no redundancy)
type RAID0 struct {
	disks []*Disk
}

func NewRAID0(disks []*Disk) *RAID0 {
	return &RAID0{disks}
}

func (r *RAID0) Write(blockNum int, data []byte) error {
	diskIndex := blockNum % len(r.disks)
	diskBlock := blockNum / len(r.disks)
	return r.disks[diskIndex].WriteBlock(diskBlock, data)
}

func (r *RAID0) Read(blockNum int) ([]byte, error) {
	diskIndex := blockNum % len(r.disks)
	diskBlock := blockNum / len(r.disks)
	return r.disks[diskIndex].ReadBlock(diskBlock)
}

// RAID 1 (mirroring)
type RAID1 struct {
	disks []*Disk
}

func NewRAID1(disks []*Disk) *RAID1 {
	return &RAID1{disks}
}

func (r *RAID1) Write(blockNum int, data []byte) error {
	for _, disk := range r.disks {
		if err := disk.WriteBlock(blockNum, data); err != nil {
			return err
		}
	}
	return nil
}

func (r *RAID1) Read(blockNum int) ([]byte, error) {
	return r.disks[0].ReadBlock(blockNum)
}

// RAID 4 (striping with dedicated parity)
type RAID4 struct {
	dataDisks  []*Disk
	parityDisk *Disk
}

func NewRAID4(disks []*Disk) *RAID4 {
	return &RAID4{
		dataDisks:  disks[:len(disks)-1],
		parityDisk: disks[len(disks)-1],
	}
}

func (r *RAID4) Write(blockNum int, data []byte) error {
	diskIndex := blockNum % len(r.dataDisks)
	diskBlock := blockNum / len(r.dataDisks)

	// Read existing blocks for parity calculation
	parity := make([]byte, BlockSize)
	for i, disk := range r.dataDisks {
		if i == diskIndex {
			continue
		}
		block, _ := disk.ReadBlock(diskBlock)
		for j := range parity {
			parity[j] ^= block[j]
		}
	}
	for j := range parity {
		parity[j] ^= data[j]
	}

	if err := r.dataDisks[diskIndex].WriteBlock(diskBlock, data); err != nil {
		return err
	}
	return r.parityDisk.WriteBlock(diskBlock, parity)
}

func (r *RAID4) Read(blockNum int) ([]byte, error) {
	diskIndex := blockNum % len(r.dataDisks)
	diskBlock := blockNum / len(r.dataDisks)
	return r.dataDisks[diskIndex].ReadBlock(diskBlock)
}

// RAID 5 (striping with distributed parity)
type RAID5 struct {
	disks []*Disk
}

func NewRAID5(disks []*Disk) *RAID5 {
	return &RAID5{disks}
}

func (r *RAID5) Write(blockNum int, data []byte) error {
	numDataDisks := len(r.disks) - 1
	stripe := blockNum / numDataDisks
	indexInStripe := blockNum % numDataDisks
	parityIndex := stripe % len(r.disks)

	dataDiskIndex := 0
	for i := 0; i < len(r.disks); i++ {
		if i == parityIndex {
			continue
		}
		if dataDiskIndex == indexInStripe {
			return r.writeStripe(stripe, parityIndex, i, data)
		}
		dataDiskIndex++
	}
	return nil
}

func (r *RAID5) writeStripe(stripe, parityIndex, writeIndex int, data []byte) error {
	parity := make([]byte, BlockSize)

	for i := 0; i < len(r.disks); i++ {
		if i == parityIndex || i == writeIndex {
			continue
		}
		block, _ := r.disks[i].ReadBlock(stripe)
		for j := range parity {
			parity[j] ^= block[j]
		}
	}

	for j := range parity {
		parity[j] ^= data[j]
	}

	if err := r.disks[writeIndex].WriteBlock(stripe, data); err != nil {
		return err
	}
	return r.disks[parityIndex].WriteBlock(stripe, parity)
}

func (r *RAID5) Read(blockNum int) ([]byte, error) {
	numDataDisks := len(r.disks) - 1
	stripe := blockNum / numDataDisks
	indexInStripe := blockNum % numDataDisks
	parityIndex := stripe % len(r.disks)

	dataDiskIndex := 0
	for i := 0; i < len(r.disks); i++ {
		if i == parityIndex {
			continue
		}
		if dataDiskIndex == indexInStripe {
			return r.disks[i].ReadBlock(stripe)
		}
		dataDiskIndex++
	}
	return nil, errors.New("invalid block number")
}

// benchmark constants
const (
	TotalSize = 50 * 1024 * 1024 // 50MB - change to simulate load
	NumBlocks = TotalSize / BlockSize
)

// generate random data
func generateData() [][]byte {
	data := make([][]byte, NumBlocks)
	for i := 0; i < NumBlocks; i++ {
		block := make([]byte, BlockSize)
		rand.Read(block)
		data[i] = block
	}
	return data
}

func benchmarkRAID(name string, raid RAID, data [][]byte) {
	fmt.Printf("Benchmarking %s:\n", name)
	fmt.Printf("Data length: %d\n", len(data))
	// Write benchmark
	start := time.Now()
	for i, block := range data {
		if err := raid.Write(i, block); err != nil {
			fmt.Printf("Write error at block %d: %v\n", i, err)
			return
		}
		// Calculate percentage of progress
		progress := (float64(i+1) / float64(len(data))) * 100

		// Display progress as percentage
		fmt.Printf("\rWrite Progress: %.2f%%", progress)
	}
	writeTime := time.Since(start)

	// Read benchmark
	start = time.Now()
	for i := 0; i < NumBlocks; i++ {
		if _, err := raid.Read(i); err != nil {
			fmt.Printf("Read error at block %d: %v\n", i, err)
			return
		}
		// Calculate percentage of progress
		progress := (float64(i+1) / float64(len(data))) * 100

		// Display progress as percentage
		fmt.Printf("\rRead Progress: %.2f%%", progress)
	}
	readTime := time.Since(start)

	fmt.Printf("\nWrite time: %v (%.2f µs/block)\n", writeTime, float64(writeTime.Microseconds())/NumBlocks)
	fmt.Printf("Read time:  %v (%.2f µs/block)\n", readTime, float64(readTime.Microseconds())/NumBlocks)
	fmt.Println()
}

// create disk files
func createDisks() ([]*Disk, error) {
	disks := make([]*Disk, NumDisks)
	for i := 0; i < NumDisks; i++ {
		disk, err := NewDisk(fmt.Sprintf("disk%d.dat", i))
		if err != nil {
			return nil, err
		}
		disks[i] = disk
	}
	return disks, nil
}

// effective load capacity
func effectiveCapacity(raidType string, numDisks int, diskSize int64) int64 {
	switch raidType {
	case "RAID 0":
		return int64(numDisks) * diskSize
	case "RAID 1":
		return diskSize
	case "RAID 4", "RAID 5":
		return int64(numDisks-1) * diskSize
	default:
		return 0
	}
}

func main() {
	data := generateData()

	// RAID 0
	disks0, _ := createDisks()
	raid0 := NewRAID0(disks0)
	benchmarkRAID("RAID 0", raid0, data)

	// RAID 1
	disks1, _ := createDisks()
	raid1 := NewRAID1(disks1)
	benchmarkRAID("RAID 1", raid1, data)

	// RAID 4
	disks4, _ := createDisks()
	raid4 := NewRAID4(disks4)
	benchmarkRAID("RAID 4", raid4, data)

	// RAID 5
	disks5, _ := createDisks()
	raid5 := NewRAID5(disks5)
	benchmarkRAID("RAID 5", raid5, data)

	//Print ELC (Effective Load Capacity)
	fmt.Println("Effective Storage Capacities:")
	fmt.Printf("RAID 0: %d MB\n", effectiveCapacity("RAID 0", NumDisks, BlockSize*NumBlocks/NumDisks)/(1024*1024))
	fmt.Printf("RAID 1: %d MB\n", effectiveCapacity("RAID 1", NumDisks, BlockSize*NumBlocks/NumDisks)/(1024*1024))
	fmt.Printf("RAID 4: %d MB\n", effectiveCapacity("RAID 4", NumDisks, BlockSize*NumBlocks/NumDisks)/(1024*1024))
	fmt.Printf("RAID 5: %d MB\n", effectiveCapacity("RAID 5", NumDisks, BlockSize*NumBlocks/NumDisks)/(1024*1024))
	fmt.Println()
}
