#include "ServerSDK/mg_Singleton.h"

#include <iostream>
using namespace std;

class A : public Singleton<A>
{
public:
    A() { a = 1, b = 1; };

    void show() { cout << a << " " << b << "\n"; };

private:
    int a, b;
};

int main()
{
    A::getMe().show();
    cout << (&A::getMe()) << endl;
    cout << (&A::getMe()) << endl;
    return 0;
}