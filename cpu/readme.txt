I use the increment in CPU time of each domain in a scheduling period as the measurement of CPU usage. 

Algorithm:
1. Record the increment in CPU time of each domain in a scheduling period
2. Sort these numbers
3. Start from the biggest number, iteratively pin the corresponding domain to the least busy physical CPU core

Example:
1. The increment in CPU time of each domain: 25000 (domain 1), 100000 (domain 2), 18000 (domain 3), 21000 (domain 4), 20000 (domain 5)
2. Sort them: 100000 (domain 2), 25000 (domain 1), 21000 (domain 4), 20000 (domain 5), 18000 (domain 3)
3. Assume we have 2 physical CPU cores, core 0 and core 1
	a) Pin domain 2 to core 0 (Loading of core 0: 100000, Loading of core 1: 0)
	b) Pin domain 1 to core 1 (Loading of core 0: 100000, Loading of core 1: 25000)
	c) Pin domain 4 to core 1 (Loading of core 0: 100000, Loading of core 1: 46000)
	d) Pin domain 5 to core 1 (Loading of core 0: 100000, Loading of core 1: 66000)
	e) Pin domain 3 to core 1 (Loading of core 0: 100000, Loading of core 1: 84000)