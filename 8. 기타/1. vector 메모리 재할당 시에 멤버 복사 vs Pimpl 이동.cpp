/*
 * https://stackoverflow.com/questions/33397967/how-to-avoid-stdvector-to-copy-on-re-allocation
 */
#include <iostream>
#include <vector>
#include <memory>
#include <chrono>

using namespace std;
using namespace std::chrono;

/*
 * vector 메모리 재할당 시에 멤버 복사 vs Pimpl 이동
 *
 * 가장 나쁜 방법: 데이터 멤버를 복사
 *
 * 가장 좋은 방법: noexcept 이동 생성자를 활용
 *
 * 생각해 볼 수 있는 방법: Pimpl 을 이용한 포인터만 이동
 * - 모든 원소가 unique_ptr 을 통해 각각 매번 동적 할당되므로
 * - 삽입/삭제 속도가 느려지고, 메모리 파편화가 심해질 수 있다.
 */

 /*
  * vector::reserve() 로 공간을 충분히 확보해놓고
  * 마지막에 vector::resize() 로 필요한 크기로 줄이면
  *
  * 재할당을 줄일 수 있다.
  */

class WidgetImpl
{
    int i;
    double b, c, d;
    string name;
    double arr[10];

    friend class Widget;

public:
    WidgetImpl(int i = 0, double b = 0.0, double c = 0.0, double d = 0.0, string name = "AAAAAAAAAAAAAABBBBBBBBBBB")
    : i(i), b(b), c(c), d(d), name(name)
    {

    }

    WidgetImpl(const WidgetImpl& rhs)
    : i(rhs.i), b(rhs.b), c(rhs.c), d(rhs.d), name(rhs.name)
    {
        //cout << "WidgetImpl::복사 생성자 " << i << endl;
    }

    /* push_back 때는 이동 생성이 되더라도,
     * 
     * noexcept 이 아니기 때문에 메모리 재할당 시에는 복사 생성이 사용된다. */
    WidgetImpl(WidgetImpl&& rhs)
        : i(move(rhs.i)), b(move(rhs.b)), c(move(rhs.c)), d(move(rhs.d)), name(move(rhs.name))
    {
        //cout << "WidgetImpl::이동 생성자 " << i << endl;
    }

    WidgetImpl& operator= (const WidgetImpl& rhs)
    {
        i = rhs.i;
        b = rhs.b;
        c = rhs.c;
        d = rhs.d;
        name = rhs.name;

        //cout << "WidgetImpl::복사 대입 연산자 " << i << endl;

        return *this;
    }
};

class Widget
{
    unique_ptr<WidgetImpl> pimpl;

public:
    Widget(int i = 0, double b = 0.0, double c = 0.0, double d = 0.0, string name = "AAAAAAAAAAAAAABBBBBBBBBBB")
    : pimpl(make_unique<WidgetImpl>(i, b, c, d, name))
    {

    }

    /* vector 는 메모리 재할당 시에
     * 이동 생성자가 noexcept 인 경우에만 복사 대신 이동을 한다. */
    Widget(Widget&& rhs) : pimpl(move(rhs.pimpl))
    {
        //cout << "Widget::이동 생성자 " << pimpl->i << endl;
    }

    Widget& operator= (const Widget& rhs)
    {
        pimpl = make_unique<WidgetImpl>(rhs.pimpl->i, rhs.pimpl->b, rhs.pimpl->c, rhs.pimpl->d, rhs.pimpl->name);

        //cout << "Widget::복사 대입 연산자 " << pimpl->i << endl;

        return *this;
    }

};

int main(int argc, char* argv[])
{
    vector<WidgetImpl> vw;
    vector<Widget> vpimpl;

    vw.push_back(WidgetImpl(1));
    //cout << "size: " << vw.size() << ", capacity: " << vw.capacity() << endl << endl;
    vw.push_back(WidgetImpl(2));
    //cout << "size: " << vw.size() << ", capacity: " << vw.capacity() << endl << endl;
    vw.push_back(WidgetImpl(3));
    //cout << "size: " << vw.size() << ", capacity: " << vw.capacity() << endl << endl;

    vw[0] = vw[1];
    //cout << endl;

    vpimpl.push_back(Widget(1));
    //cout << "size: " << vpimpl.size() << ", capacity: " << vpimpl.capacity() << endl << endl;
    vpimpl.push_back(Widget(2));
    //cout << "size: " << vpimpl.size() << ", capacity: " << vpimpl.capacity() << endl << endl;
    vpimpl.push_back(Widget(3));
    //cout << "size: " << vpimpl.size() << ", capacity: " << vpimpl.capacity() << endl << endl;

    vpimpl[0] = vpimpl[1];
    //cout << endl;

    system_clock::time_point start, end;
    vw.clear();
    vpimpl.clear();
    
    /*
     * 크기 증가로 메모리 재할당 시에
     * 데이터 멤버를 직접 복사하는 버전
     * 
     * 약 21초 소요
     * 
     * 재할당 시에 원본 데이터 크기만큼의
     * 추가 공간이 필요하므로
     * 
     * 메모리 재할당이 일어날 때마다
     * 메모리 사용량이 들쑥날쑥 해진다.
     */
    {
        start = system_clock::now();

        for (int i = 0; i < 3000000; ++i)
            vw.push_back(WidgetImpl(i));

        end = system_clock::now();
        cout << duration<double>(end - start).count() << " 초" << endl;
    }

    /*
     * 크기 증가로 메모리 재할당 시에
     * Pimpl 포인터만 이동하는 버전
     *
     * 약 15초 소요
     *
     * 재할당 시에 포인터 크기만큼의
     * 추가 공간만 필요하므로
     *
     * 메모리 사용량이 선형적으로 증가한다.
     */
    {
        start = system_clock::now();

        for (int i = 0; i < 3000000; ++i)
            vpimpl.push_back(Widget(i));

        end = system_clock::now();
        cout << duration<double>(end - start).count() << " 초" << endl;
    }

    return 0;
}
