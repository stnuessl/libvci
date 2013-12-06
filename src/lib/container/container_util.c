

unsigned int adjust(unsigned int m, unsigned int min)
{
    unsigned int n;
    
    if(m < min)
        return min;
    
    n = 1;
    
    while(n < m)
        n <<= 1;
    
    return n;
}