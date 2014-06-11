
unsigned int getMaxPrime(unsigned int num) {
	unsigned int n;
	unsigned int m;
	unsigned int k;
	unsigned int p = 0;
	int loopEnd = num / 6 + 1;
        for(n=loopEnd;n>=2;n--) {
                for(m=0;m<=1;m++) {
			int tmp = 2 * (3*n+m) - 1;
                        int find = 1;
			if (tmp < 2 || tmp > num) {
				goto label1;
			}
			for (k=2;k*k<=tmp;k++) {
				if (tmp % k == 0) {//不是质数
                                        
					if (m == 0) {
                                                find = 0;
						goto label2;//继续内层循环
					} else {
						goto label1;//继续外层循环
					}
				}
			}
                        label2:
                        if (find == 1) {
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
