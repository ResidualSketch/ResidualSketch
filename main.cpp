#include <cassert>
// #include "HHBench.h"
// #include "HCBench.h"
#include "HHHTest.h"
#include "dataset_measure.h"
#include "dataset_analysis.h"
#include "dataset_measure_2D.h"

std::string folder = "./datasets/skewness/";

std::string file[6] = {"../caida.dat", "caida_1D_src_10.dat", "caida_1D_src_20.dat", "caida_1D_src_30.dat","caida_1D_src_40.dat","caida_1D_src_50.dat"};
std::string name[6] = {"caida", "caida_10", "caida_20", "caida_30","caida_40","caida_50"};

int main(int argc,char* argv[]) {
    if (argc != 2)
    {
        std::cout << "Parameters Error!" << std::endl;
        return 0;
    }

    int exp_type = atoi(argv[1]);
    if(exp_type == 1){
        std::cout << "HHHMeasure" << std::endl;
        for(int k=0;k<=5;k++)
            for(int j=0;j<=1;j++){
                HHHMeasure HM(folder + file[k], name[k], 33, j);
                for(uint32_t i = 1;i <= 10;++i){
                    HM.OutputHHH(i * 10000);
                } 
                
            }
    }else if(exp_type == 2){
        std::cout << "HHH Full Key Measure" << std::endl;
        for(int k=0;k<=5;k++)
            for(uint32_t i = 1;i <= 10;++i){
                dataset_analysis(folder + file[k], name[k], i* 10000);
        } 
                
    }else if(exp_type == 3){
        std::cout << "HHHTest_byte" << std::endl;
        for(int k = 0; k <= 5; k++){
            for(int j = 0; j <= 1; j++){
                HHHTEST HT1(folder + file[k], name[k], 5, j);
                for(uint32_t i = 1;i <= 10;++i){
                    HT1.HHH_HHH12(i*10000);
                    HT1.HHHFullAncestry(i*10000);
                    HT1.HHHRHHH(i*10000);
                    HT1.HHHMVPipe(32*1024, i*10000);
                    HT1.HHHKey(32*1024, i*10000);
                }
            }
        }
    }else if(exp_type == 4){
        std::cout << "HHHTest" << std::endl;
        for(int k = 0; k <= 5; k++){
            for(int j = 0; j <= 1; j++){
                HHHTEST HT(folder + file[k], name[k], 33, j);
                for(uint32_t i = 1;i <= 10;++i){
                    HT.HHH_HHH12(i*10000);
                    HT.HHHFullAncestry(i*10000);
                    HT.HHHRHHH(i*10000);
                    HT.HHHMVPipe(256*1024, i*10000);
                    HT.HHHUSS(256*1024,i*10000);
                    HT.HHHCOCO(256*1024, i*10000);
                    HT.HHHKey(256*1024, i*10000);
                    HT.HHHKey_three(256*1024, i*10000);
                    HT.HHHKey_four(256*1024, i*10000);
                }
            }
        }
    }



    return 0;
}