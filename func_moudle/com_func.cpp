#include <iostream>
#include "com_func.h"
using namespace std;

com_func::com_func(/* args */)
{
}

com_func::~com_func()
{
}

void com_func::my_str(){
    /*********find()*******/
    string name("AnnaLiviaPlurabelle");
    typedef string::size_type size_type; //貌似也是迭代器的一种
    size_type startPos = name.find('L');//像是返回了一个偏移量，因为erase用到的参数是startPos + name.begin()
    size_type endPos = name.find_last_of('a');//找最后一个a出现的位置

    /*********erase()******/
    name.erase(name.begin()+startPos, name.begin()+endPos+1);//name.begin()+endPos+1,加1是为了把那个位置上的a一块删了,否则用下面的一条语句删掉多余的a
    //name.earse(endPos);
    cout << name << endl;

    /********insert()******/
    string string_object( "Missisippi" );
    size_type pos = string_object.find("isi");//可以是char *,或者是char [],返回的具体是那个位置，输出看看

    string_object.insert(pos+1, "s");

    cout << string_object << endl;

    //将一个新的字符串inert进去
    string new_string = "AnnaBelle Lee";
    size_type pos2 = new_string.find('B');
    size_type endPos2 = new_string.find(' ');

    //cout << string_object.size() << endl;// string_object.size()是11，貌似是字符串大小
    string_object.insert(string_object.size(), //string_object中的位置
                        new_string, pos2,     //new_string的开始位置
                        endPos2-pos);          //要拷贝字符的数目

    cout << string_object << endl;

    /**********asign()和append()*****/
    string s1( "Mississippi" );
    string s2( "Annabelle" );

    string s3;
    //拷贝s1前四个字符，即从0位置，共拷贝4个字符
    s3.assign(s1, 0, 4);//s3 现在的值为 Miss
    //连接s2的前4个字符
    s3.append(s2, 0, 4);//s3 现在的值为 MissAnna
    
    //也可以合在一块写
    //s3.assign( s1, 0, 4 ).append( ' ' ).append( s2, 0, 4 );

    //赋值belle
    string beauty;
    beauty.assign(s2, 4, 5);

    //另外一种形式则不用提供位置和长度 而是提供一个 iterator 对
    //beauty.assign(s2, s2.begin()+4, s2.end());

    /*************************swap()****************/
    //会交换两个 string 对象的值
    string current_project( "C++ Primer, 3rd Edition" );
    string pending_project( "Fantasia 2000, Firebird segment" );

    current_project.swap( pending_project );

    /***********************下标操作*********/
    string first_novel( "V" );
    char ch = first_novel[0];
    cout << ch << endl;

    /***********************compare()*****/
}