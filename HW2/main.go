package main

import (
	"fmt"
	"sync"
	"sync/atomic"
	"time"
)

/* Ticket Lock Structure*/
type TicketLock struct {
	ticket uint32
	turn   uint32
}

/*Ticket Lock Lock function*/
func (lock *TicketLock) Lock() {
	myTurn := atomic.AddUint32(&lock.ticket, 1) - 1
	for atomic.LoadUint32(&lock.turn) != myTurn {
		/*---Spin---*/
	}
}

/*Ticket Lock Unlock function*/
func (lock *TicketLock) Unlock() {
	atomic.AddUint32(&lock.turn, 1) /*Increment ticket*/
}

/*Compare-and-Swap Spin Lock Structure*/
type CASLock struct {
	locked uint32
}

/*Compare-and-Swap Spin Lock Lock Function*/
func (lock *CASLock) Lock() {
	for !atomic.CompareAndSwapUint32(&lock.locked, 0, 1) {
		/*---Spin---*/
	}
}

/*Compare-and-Swap Spin Lock Unlock Function*/
func (lock *CASLock) Unlock() {
	atomic.StoreUint32(&lock.locked, 0)
}

/*------Benchmarking function----------*/
func benchmarkLock(lock interface {
	Lock()
	Unlock()
}, goroutines int, iterations int) int64 {
	var wg sync.WaitGroup             /*Sync For GoRoutines*/
	start := time.Now()               /*Capture Time*/
	for i := 0; i < goroutines; i++ { /*Create Go Routines*/
		wg.Add(1) /*Add to Wait Group*/
		/*--------*/
		go func() {
			for j := 0; j < iterations; j++ {
				lock.Lock()
				/*Critical section (simulate some work)*/
				for j := 0; j < 10; j++ {
				}
				lock.Unlock()
			}
			wg.Done() /*Done signal*/
		}()
	}

	wg.Wait()
	end := time.Now() /*Capture End Time*/

	return end.Sub(start).Nanoseconds() /*Calculate Time*/
}

func main() {
	goroutines := []int{2, 4, 8, 16} /*Number of goroutines per benchmark*/
	iterations := 1000               /*number of iterations for lock calls*/

	for _, g := range goroutines {
		tLock := &TicketLock{} /*create Ticket Lock*/
		cLock := &CASLock{}    /*create Comp. & Swap Lock*/

		tDuration := benchmarkLock(tLock, g, iterations) /*Run tests for Ticket Lock*/
		cDuration := benchmarkLock(cLock, g, iterations) /*Run tests for Comp. & Swap Lock*/

		/*----Print Results--------*/
		fmt.Printf("Goroutines: %d\n", g)
		fmt.Printf("Ticket Lock Duration: %d nanoseconds\n", tDuration)
		fmt.Printf("CAS Lock Duration: %d nanoseconds\n", cDuration)
		fmt.Printf("-----------------------------\n")
	}
}
