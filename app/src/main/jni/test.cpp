//
// Created by Administrator on 2017/1/22.
//
#include <iostream>
#include <android/log.h>

static void print_debug(const char *msg) {
//#ifdef DEBUG
    __android_log_print(ANDROID_LOG_ERROR, "JNITag", "%s", msg);
//#endif
}

using namespace std;

//声明基类Student
class Student {
public:
    Student(string);  //声明构造函数
    virtual void display();//声明输出函数
protected:  //受保护成员，派生类可以访问

    string name;

};

//Student类成员函数的实现
Student::Student(string nam)//定义构造函数
{
    name = nam;
}

void Student::display()//定义输出函数
{
//cout << "num:" << num << "\nname:" << name << "\nscore:" << score << "\n\n";
    print_debug("stu display");
}

//声明公用派生类Graduate
class Graduate : public Student {
public:
    Graduate(string);

    //声明构造函数
    void display();//声明输出函数
private:
    float pay;
};

// Graduate类成员函数的实现
void Graduate::display()//定义输出函数
{
    print_debug("Graduate display");
}

Graduate::Graduate(string nam) : Student(nam) { }
//主函数

void test1() __attribute__((constructor));

void test1() {
    Student stud1("Li");//定义Student类对象stud1
    Graduate grad1("Wang");//定义Graduate类对象grad1
    Student *pt = &stud1;//定义指向基类对象的指针变量pt
    pt->display();
    pt = &grad1;
    pt->display();
}