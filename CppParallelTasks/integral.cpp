#include<iostream>

using namespace std;

float f(float(x)) {
    return (x*x)/2;
}


int main() {
    double a,b,d,n,I=0,J=0,A;
    cout<<"Given f(x)= x"<<endl;
    cout<<"Enter lower limit :"<<endl;
    cin>>a;
    cout<<"Enter Upper Limit :"<<endl;
    cin>>b;
    cout<<"Enter the number of intervals : "<<endl;
    cin>>n;
    d=(b-a)/n;


    for(int i=0;i<=n;i++) {
        I=I+f(a+(i*d));
    }

    for(int i=1;i<n;i++) {
        J=J+f(a+(i*d));
    }
    A=(d/2)*(I+J);


    cout<<"The Value of integral under the enterd limits is : "<<endl;
    cout<<A<<endl;

    return 0;
}


