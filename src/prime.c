
unsigned int getMaxPrime(unsigned int num)
{
    unsigned int n;
    unsigned int m;
    unsigned int k;
    unsigned int p = 0;
    int loopEnd = num / 6 + 1;
	if (loopEnd < 3) {
        return 7;
    }
    for(n = loopEnd; n >= 2; n--)
    {
        for(m = 0; m <= 1; m++)
        {
            unsigned int tmp = 2 * (3 * n + m) - 1;//6n+-1

            int find = 1;
            if (tmp > num) {
            	goto label1;
            }
            if (tmp < 2)
            {
                continue;
            }
            for (k = 2; k * k <= tmp; k++)
            {
                if (tmp % k == 0)  //不是质数
                {

                    if (m == 0)
                    {
                        find = 0;
                        goto label2;//继续内层循环
                    }
                    else
                    {
                        goto label1;//继续外层循环
                    }
                }
            }
label2:
            if (find == 1)
            {
                p = tmp;
                goto end;//结束所有循环
            }

            //return p;
        }
label1:
        ;//
    }
end:
    return p;
}
