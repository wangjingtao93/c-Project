#include <vector>
#include <iostream>

#include "my_stl.h"

using namespace std;

my_stl::my_stl(/* args */)
{
}

my_stl::~my_stl()
{
}

void my_stl::icapacity()
{
    vector<int> ivec;
    cout << "ivec: size: " << ivec.size()
         << " capacity: " << ivec.capacity() << endl;

    for (int ix = 0; ix < 24; ++ix)
    {
        ivec.push_back(ix);
        cout << "ivec: size: " << ivec.size()
             << " capacity: " << ivec.capacity() << endl;
    }
}


void my_stl::even_odd(const vector<int> *pvec, vector<int> *pvec_even, vector<int> *pvec_odd){
    //必须声明一个const_iterator,才能遍历pvec
    vector<int>::const_iterator c_it = pvec->begin();
    for(;c_it != pvec->end(); ++c_it){
        if(*c_it % 2)
            pvec_even->push_back(*c_it);
        else
        {
            pvec_odd->push_back(*c_it);
        }   
    }
}



