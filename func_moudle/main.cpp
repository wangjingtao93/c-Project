#include <stdio.h>
#include "timestamp_ms.h"
#include <iostream>
#include <memory>

#include "raptorkv.h"
#include "my_stl.h"
#include "com_func.h"

#include <vector>

using namespace std;

int main(int argc, char* argv[]){
    // int i;
    // for(i = 0; i < argc; i++){
    //     //argc的大小就是你敲得命令参数的个数，包括执行文件（argv[0]就是执行文件名）
    //     printf("Argumeng argv[%d]:%s\n",i, argv[i]);
    // }

    // uint64_t now_ms = utils::now_timestamp_ms();//返回当前时间，秒级
    // std::cout << now_ms << std::endl;

    // //智能指针牛逼
    // //std::shared_ptr<RaptorKV> kv_instance = std::make_shared<RaptorKV>(table_name, raft_conf);
    // std::shared_ptr<raptorkv> kv_instance = std::make_shared<raptorkv>(); 
    // kv_instance->init();

    /***********my_stl**********/
    // 输出结果有点意思
    // my_stl i_stl;
    // i_stl.icapacity();


    
    // 输出结果有点意思
    const int ia[] = {1,2,3,4,5,6,7,8,9,10};
    const vector<int> pvec(ia, ia+10);

    vector<int> pvec_even;
    vector<int> pvec_old;

    my_stl i_stl;
    i_stl.even_odd(&pvec, &pvec_even, &pvec_old);
    
    for(vector<int>::iterator it=pvec_even.begin();it !=pvec_even.end();++it){
        cout << *it << endl;
    }
    /*******************end*********************/


    /**************com_func********************/
    com_func *p_com = new com_func();

    p_com->my_str();



    delete p_com;
    

    /*******************end*********************/



    return 0;
}