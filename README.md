# Sports-Scheduling-Simulator

This system implements a multi-threaded simulator that coordinates multiple sports teams sharing the same field. It ensures fairness, safety, and concurrency constraints while preventing thread starvation.

It uses the following features:
- multi-threaded simulation of multiple sports being scheduled on the same field
- thread synchronization using mutexes, conditional variables, and semaphores
- scheduling without a centralized coordinator
