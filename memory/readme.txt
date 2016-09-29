I try to balance the free memory of each domain.  

Algorithm:
1. Record the free memory of each domain (value of tag 4). 
2. Calculate an average number. (free_mem_1 + free_mem_2 + free_mem_3 + ... + freemem_n + buffer) / n
3. Adjust the memory assigned to each domain to make their free memory same. 
	a) If the amount of memory I want to assign to a domain exceed its max memory, I would put the exceeding part into a buffer, 
	b) Then add the buffer back to calculate the average free memory in the next round. 
	c) If I did not use the buffer, the average memory of each domain will become less and less, since the part which exceeds the max memory of a domain would just gone forever. 

Example:
1. free_mem_1 = 80000, free_mem_2 = 80000, free_mem_3 = 80000, free_mem_4 = 20000, buffer = 20000
2. Avg free memory = (80000 + 80000 + 80000 + 20000 + 20000) / 4 = 70000
3. Original memory assigned to each domain is 90000, max memory is 100000
	a) Decrease the memory of domain 1 from 90000 to 90000 - (80000 - 70000) = 80000
	b) Decrease the memory of domain 2 from 90000 to 90000 - (80000 - 70000) = 80000
	c) Decrease the memory of domain 3 from 90000 to 90000 - (80000 - 70000) = 80000
	d) Increase the memory of domain 4 from 90000 to 90000 - (20000 - 70000) = 140000
		i) But 140000 > max memory, which is 100000
		ii) So we assign 100000 to domain 4, and put the exceeding part 140000 - 100000 = 40000 into the buffer