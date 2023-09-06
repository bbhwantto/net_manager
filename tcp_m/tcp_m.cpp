#include "tcp_interface.h"
#include "r_w.h"
#include "requestData.h"
vector<node> blacklist;
Itcp_manager* create_tcp_mgr()
{
	Itcp_manager* it = new(Itcp_manager_ach);
	return it;
}

Connect_tyep c_type;
const char* file;
int main()
{

	int n;
	Itcp_manager* it = create_tcp_mgr();
	cout << "创建类型" << endl;
	cout << "1 : CS" << endl;
	cout << "2 : BS" << endl;

	cin >> n;
	if (n == 1)
	{
		c_type = H_CS;
	}
	else if (n == 2)
	{
		c_type = H_BS;
	}
	else
	{
		cout << "输入错误" << endl;
		return 0;
	}
	file = "./root/4.mp3";
	n = 12;
	it->init(n);

	Sleep(2000);
	//it->release();
	char p;
	cin >> p;
	return 0;
}
