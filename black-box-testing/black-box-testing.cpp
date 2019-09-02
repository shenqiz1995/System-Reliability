// black-box-testing.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "stdio.h"
#include "stdlib.h"
#include "malloc.h"
#include "math.h"
#include "time.h"
#include "iostream"
#include "stack"
#include "queue"
using namespace std;
#define MAXN_process 100        //过程结点数上限
#define MAXN_function 100        //功能结点数上限
#define MAXN_machine 100        //机器结点数上限
#define MAXN_component 100        //零件结点数上限
#define MAXN_resource 100        //资源结点数上限
#define MAXN_row 45        //标准正态分布表行数
#define MAXN_column 10        //标准正态分布表列数
#define MAXN_time 100        //测试时间上限
#define MAXN_backup_place 3        //存储地点数上限
#define MAXN_repair_place 2        //修复地点数上限


typedef struct linked_list
{
	int node_num;        //结点编号
	int node_sum;        //结点数目
	struct linked_list *next;        //指针
} Linked_List;
Linked_List *adjacency_list_forward[MAXN_process + MAXN_function];        //自顶向下的邻接表
Linked_List *adjacency_list_reverse[MAXN_process + MAXN_function];        //自底向上的邻接表
bool adjacency_matrix_forward[MAXN_process + MAXN_function][MAXN_process + MAXN_function];        //自顶向下的邻接矩阵

struct
{
	int in_degree;        //入度
	int out_degree;        //出度
	bool fault;        //当前状态（0为没有故障，1为发生故障）
	bool visited;        //是否已经访问过（0为未访问过，1为访问过）
} node[MAXN_process + MAXN_function];

struct
{
	bool removable;        //是否可以移除（0表示不可移除，1表示可以移除）
	bool removed;        //是否已经被移除（0表示还未被移除，1表示已经被移除）
	int logic_gate;        //逻辑门（1为与门，2为或门，3为表决门，4为异或门，5为同或门）
	int least_event;        //表决门最少发生的事件数
} process[MAXN_process];

struct
{
	double capacity;        //能力
	Linked_List *machine;        //所需的机器
} function[MAXN_function];

struct
{
	//可靠度属性
	double reliability[MAXN_time];        //初始的可靠度
	int reliability_move_step;        //可靠度曲线相对于初始的可靠度曲线需要移动的步长

	//能力属性
	double initial_capacity;        //初始能力
	double capacity;        //能力
	bool fault;        //当前状态（0为没有故障，1为发生故障）

	//备份属性
	int backup_initial_sum[MAXN_backup_place];        //初始的备份数目（不包括当前正在工作的机器）
	int backup_delay;        //备份延迟时间（热备份延迟时间为0，温备份和冷备份延迟时间大于0）
	double backup_capacity_ratio;        //处在延迟时间内的机器的能力比例（温备份比例大于0，冷备份比例为0）
	int backup_recover_time_remaining;        //当前正在工作的机器距离正常运行的时间

	//修复属性
	double repair_reliability;        //进行修复时的可靠度（维修性修复可靠度为0，预防性修复可靠度大于0）
	double repair_unrecoverable_rate;        //报废率

	//零件属性
	Linked_List *component;        //所需的零件

	//使用属性
	bool used;        //是否被使用
} machine[MAXN_machine];

struct
{
	//备份属性
	int backup_initial_sum[MAXN_backup_place];        //初始的备份数目（不包括当前正在工作的零件）
	int backup_available_sum[MAXN_backup_place];        //可以使用的备份数目（不包括当前正在工作的零件）
	int backup_minimum_sum[MAXN_backup_place - 1];        //最少的备份数目
	queue<int> backup_arrive_time_remaining[MAXN_backup_place];        //备份距离送达的时间

	//修复属性
	int repair_time;        //修复时间
	int repair_place;        //修复地点（0为在基层修复，1为在中继修复）
	queue<int> repair_recover_time_remaining;        //损坏的零件距离修复好的时间
	queue<int> repair_arrive_time_remaining;        //损坏的零件距离送达修复地点的时间

	//资源属性
	Linked_List *resource;        //所需的资源

	//存储和运输属性
	double volume;        //体积
	double weight;        //重量
	double storage_cost_per_volume[MAXN_backup_place];        //备份单位体积的储存费用
} component[MAXN_component];

struct
{
	//备份属性
	int backup_initial_sum[MAXN_repair_place];        //初始的备份数目
	int backup_available_sum[MAXN_repair_place];        //可以使用的备份数目

	//修复属性
	double repair_cost_per_time_unit;        //工时费用
} resource[MAXN_resource];

struct
{
	int time[MAXN_backup_place][MAXN_backup_place];        //运输时间
	double cost;        //运输费用
} transport;

struct
{
	int fault[MAXN_process][MAXN_time];        //故障次数
	double capacity[MAXN_time];        //能力
	double cost;        //花费
} result;

int N_process;        //过程结点数
int N_process_exist;        //未被移除的过程结点数
int N_function;        //功能结点数
int N_machine;        //机器结点数
int N_component;        //零件结点数
int N_resource;        //资源结点数
double standard_normal_distribution_table[MAXN_row][MAXN_column];        //标准正太分布表
int time_sum = MAXN_time;
int test_sum = 100;
int preventive_repair_time = 8;        //进行预防性修复的间隔
int backup_place_sum = MAXN_backup_place;        //存储地点数
int repair_place_sum = MAXN_repair_place;        //修复地点数
stack<int> node_to_remove;        //在一次深度优先搜索中将会被移除的结点
stack<int> node_to_reconnect;        //在一次深度优先搜索中将会被重新连接的结点
stack<int> node_to_reconsider;        //在一次深度优先搜索中可能导致再次深度优先搜索的结点
int sequence_process[MAXN_process];        //遍历过程结点的顺序
queue<int> component_waiting_for_repair[MAXN_repair_place];        //等待修复的零件


/**************************************************对链表的基本操作**************************************************/


//在链表头加入一个结点
Linked_List *insert_node_to_list(Linked_List *list, int node_num, int node_sum)
{
	Linked_List *p;
	p = (Linked_List*)malloc(sizeof(Linked_List));
	p->node_num = node_num;
	p->node_sum = node_sum;
	p->next = list;
	return p;
}


//在链表中移除一个结点
Linked_List *remove_node_from_list(Linked_List *list, int node_num)
{
	Linked_List *p, *q;
	bool flag;
	p = list;
	q = p->next;
	flag = 1;
	if (list->node_num == node_num)
	{
		p = list;
		list = list->next;
		free(p);
		flag = 0;
	}
	while (q != NULL&&flag == 1)
	{
		if (q->node_num == node_num)
		{
			p->next = q->next;
			free(q);
			flag = 0;
		}
		else
		{
			p = q;
			q = q->next;
		}
	}
	return list;
}


//销毁链表
Linked_List *destroy_list(Linked_List *list)
{
	Linked_List *p;
	p = list;
	while (p != NULL)
	{
		list = p->next;
		free(p);
		p = list;
	}
	return list;
}


/**************************************************读入参数**************************************************/


//连接结点
void connect_nodes(int input, int output)
{
	adjacency_list_forward[output] = insert_node_to_list(adjacency_list_forward[output], input, 1);
	adjacency_list_reverse[input] = insert_node_to_list(adjacency_list_reverse[input], output, 1);
	adjacency_matrix_forward[output][input] = 1;
	node[output].in_degree++;
	node[input].out_degree++;
}


//读取结点数
void fscan_N()
{
	FILE *fp = fopen("input N.txt", "r");
	fscanf(fp, "%d%d%d%d%d", &N_process, &N_function, &N_machine, &N_component, &N_resource);
	fclose(fp);
}


//读取网络
void fscan_network()
{
	int i, j;
	FILE *fp = fopen("input network.txt", "r");
	int input, output;
	int input_sum;
	for (i = 0; i < N_process; i++)
	{
		fscanf(fp, "%d%d", &output, &input_sum);

		for (j = 0; j < input_sum; j++)
		{
			fscanf(fp, "%d", &input);
			connect_nodes(input, output);
		}
	}
	fclose(fp);
}


//读入过程结点
void fscan_process()
{
	int i;
	FILE *fp = fopen("input process.txt", "r");
	int node_num;
	for (i = 0; i < N_process; i++)
	{
		fscanf(fp, "%d", &node_num);
		fscanf(fp, "%d%d", &process[node_num].removable, &process[node_num].logic_gate);

		//如果逻辑门是表决门
		if (process[node_num].logic_gate == 3)
		{
			fscanf(fp, "%d", &process[node_num].least_event);
		}
	}
	fclose(fp);
}


//读入功能结点
void fscan_function()
{
	int i, j;
	FILE *fp = fopen("input function.txt", "r");
	int node_num;
	int machine_sum;
	int machine_num;
	stack<int> node_input;
	for (i = 0; i < N_function; i++)
	{
		fscanf(fp, "%d%d", &node_num, &machine_sum);
		for (j = 0; j < machine_sum; j++)
		{
			fscanf(fp, "%d", &machine_num);
			node_input.push(machine_num);
		}
		while (node_input.empty() == 0)
		{
			function[node_num].machine = insert_node_to_list(function[node_num].machine, node_input.top(), 1);
			node_input.pop();
		}
	}
	fclose(fp);
}


//读入标准正态分布表
void fscan_standard_normal_distribution_table()
{
	int i, j;
	FILE *fp = fopen("input standard normal distribution table.txt", "r");
	int row_sum = MAXN_row;
	int column_sum = MAXN_column;
	for (i = 0; i < row_sum; i++)
	{
		for (j = 0; j < column_sum; j++)
		{
			fscanf(fp, "%lf", &standard_normal_distribution_table[i][j]);
		}
	}
	fclose(fp);
}


//计算标准正态分布函数值
double standard_normal_distribution_function(double x)
{
	double result;
	bool negative = 0;
	if (x < 0)
	{
		negative = 1;
		x = -x;
	}

	x = round(x * 100)*1.0 / 100;

	if (x >= 4.5)
	{
		result = 1;
	}
	else
	{
		int row, col;
		row = floor(x * 10);
		col = int((x * 10 - floor(x * 10)) * 10);
		result = standard_normal_distribution_table[row][col];
	}

	//若为负数
	if (negative == 1)
	{
		result = 1 - result;
	}

	return result;
}


//读入机器结点
void fscan_machine()
{
	int i, j;
	Linked_List *p;
	FILE *fp1 = fopen("input machine reliability.txt", "r");
	FILE *fp2 = fopen("input machine capacity.txt", "r");
	FILE *fp3 = fopen("input machine backup.txt", "r");
	FILE *fp4 = fopen("input machine repair.txt", "r");
	FILE *fp5 = fopen("input machine component.txt", "r");
	int node_num;
	int distribution_type;        //分布（1为指数分布，2为正态分布，3为对数正态分布，4为威布尔分布）
	double theta, mu, sigma, eta, m;
	int component_type_sum, component_num, component_sum;

	//读入可靠度属性
	for (i = 0; i < N_machine; i++)
	{
		fscanf(fp1, "%d%d", &node_num, &distribution_type);

		//指数分布
		if (distribution_type == 1)
		{
			fscanf(fp1, "%lf", &theta);
			for (j = 0; j < time_sum; j++)
			{
				machine[node_num].reliability[j] = exp(-j*1.0 / theta);
			}
		}

		//正态分布
		else if (distribution_type == 2)
		{
			fscanf(fp1, "%lf%lf", &mu, &sigma);
			for (j = 0; j < time_sum; j++)
			{
				machine[node_num].reliability[j] = 1 - standard_normal_distribution_function((j*1.0 - mu) / sigma);
			}
		}

		//对数正态分布
		else if (distribution_type == 3)
		{
			fscanf(fp1, "%lf%lf", &mu, &sigma);
			for (j = 0; j < time_sum; j++)
			{
				machine[node_num].reliability[j] = 1 - standard_normal_distribution_function((log(j*1.0) - mu) / sigma);
			}
		}

		//威布尔分布
		else if (distribution_type == 4)
		{
			fscanf(fp1, "%lf%lf", &eta, &m);
			for (j = 0; j < time_sum; j++)
			{
				machine[node_num].reliability[j] = exp(-pow(j*1.0 / eta, m));
			}
		}
	}

	//读入能力属性
	for (i = 0; i < N_machine; i++)
	{
		fscanf(fp2, "%d", &node_num);
		fscanf(fp2, "%lf", &machine[node_num].initial_capacity);
	}

	//读入零件属性
	for (i = 0; i < N_machine; i++)
	{
		fscanf(fp5, "%d%d", &node_num, &component_type_sum);
		for (j = 0; j < component_type_sum; j++)
		{
			fscanf(fp5, "%d%d", &component_num, &component_sum);
			machine[i].component = insert_node_to_list(machine[i].component, component_num, component_sum);
		}
	}

	//读入备份属性
	for (i = 0; i < N_machine; i++)
	{
		fscanf(fp3, "%d", &node_num);
		for (j = 0; j < backup_place_sum; j++)
		{
			fscanf(fp3, "%d", &machine[node_num].backup_initial_sum[j]);

			//更新零件的备份数
			p = machine[node_num].component;
			while (p != NULL)
			{
				component[p->node_num].backup_initial_sum[j] += machine[node_num].backup_initial_sum[j];
				p = p->next;
			}
		}
		fscanf(fp3, "%d%lf", &machine[node_num].backup_delay, &machine[node_num].backup_capacity_ratio);
	}

	//读入修复属性
	for (i = 0; i < N_machine; i++)
	{
		fscanf(fp4, "%d", &node_num);
		fscanf(fp4, "%lf%lf", &machine[node_num].repair_reliability, &machine[node_num].repair_unrecoverable_rate);
	}

	fclose(fp1);
	fclose(fp2);
	fclose(fp3);
	fclose(fp4);
	fclose(fp5);
}


//读入零件结点
void fscan_component()
{
	int i, j;
	Linked_List *p;
	FILE *fp1 = fopen("input component backup.txt", "r");
	FILE *fp2 = fopen("input component repair.txt", "r");
	FILE *fp3 = fopen("input component resource.txt", "r");
	FILE *fp4 = fopen("input component storage transport.txt", "r");
	int node_num;
	double backup_ratio;
	int resource_type_sum, resource_num, resource_sum;

	//读入备份属性
	for (i = 0; i < N_component; i++)
	{
		fscanf(fp1, "%d", &node_num);
		for (j = 0; j < backup_place_sum - 1; j++)
		{
			fscanf(fp1, "%lf", &backup_ratio);
			component[i].backup_minimum_sum[j] = int(component[i].backup_initial_sum[j] * backup_ratio);
		}
	}

	//读入修复属性
	for (i = 0; i < N_component; i++)
	{
		fscanf(fp2, "%d", &node_num);
		fscanf(fp2, "%d%d", &component[node_num].repair_time, &component[node_num].repair_place);
	}

	//读入资源属性
	for (i = 0; i < N_component; i++)
	{
		fscanf(fp3, "%d%d", &node_num, &resource_type_sum);
		for (j = 0; j < resource_type_sum; j++)
		{
			fscanf(fp3, "%d%d", &resource_num, &resource_sum);
			component[i].resource = insert_node_to_list(component[i].resource, resource_num, resource_sum);
		}
	}

	//读入存储和运输属性
	for (i = 0; i < N_component; i++)
	{
		fscanf(fp4, "%d", &node_num);
		fscanf(fp4, "%lf%lf", &component[node_num].volume, &component[node_num].weight);
		for (j = 0; j < backup_place_sum; j++)
		{
			fscanf(fp4, "%lf", &component[node_num].storage_cost_per_volume[j]);
		}
	}

	fclose(fp1);
	fclose(fp2);
	fclose(fp3);
	fclose(fp4);
}


//读入资源结点
void fscan_resource()
{
	int i, j;
	FILE *fp1 = fopen("input resource backup.txt", "r");
	FILE *fp2 = fopen("input resource repair.txt", "r");
	int node_num;

	//读入备份属性
	for (i = 0; i < N_resource; i++)
	{
		fscanf(fp1, "%d", &node_num);
		for (j = 0; j < repair_place_sum; j++)
		{
			fscanf(fp1, "%d", &resource[node_num].backup_initial_sum[j]);
		}
	}

	//读入修复属性
	for (i = 0; i < N_resource; i++)
	{
		fscanf(fp2, "%d", &node_num);
		fscanf(fp2, "%lf", &resource[node_num].repair_cost_per_time_unit);
	}

	fclose(fp1);
	fclose(fp2);
}


//读入运输参数
void fscan_transport()
{
	int i, j, k;
	FILE *fp = fopen("input transport.txt", "r");

	//读入相邻两个存储地点间的运输时间
	for (i = 0; i < backup_place_sum - 1; i++)
	{
		fscanf(fp, "%d", &transport.time[i][i + 1]);
		transport.time[i + 1][i] = transport.time[i][i + 1];
	}

	//生成运输时间矩阵
	for (i = 0; i < backup_place_sum - 2; i++)
	{
		for (j = 0; j < backup_place_sum - i - 2; j++)
		{
			transport.time[j][j + i + 2] = transport.time[j][j + i + 1] + transport.time[j + i + 1][j + i + 2];
			transport.time[j + i + 2][j] = transport.time[j][j + i + 2];
		}
	}

	//读入花费
	fscanf(fp, "%lf", &transport.cost);

	fclose(fp);
}


/**************************************************化简网络**************************************************/


//更新visited
void refresh_visited()
{
	int i;
	for (i = 0; i < N_process + N_function; i++)
	{
		node[i].visited = 0;
	}
}


//深度优先搜索
void depth_first_search(int node_num)
{
	Linked_List *p;
	node[node_num].visited = 1;
	p = adjacency_list_forward[node_num];
	while (p != NULL)
	{
		if (node[p->node_num].visited == 0)
		{
			//如果遇到功能结点
			if (p->node_num >= N_process)
			{
				node[p->node_num].visited = 1;
				node_to_reconnect.push(p->node_num);
			}

			//如果遇到过程结点
			else
			{
				//如果遇到逻辑门不一致或是不可移除的结点
				if (process[p->node_num].logic_gate != process[node_num].logic_gate || process[p->node_num].removable == 0)
				{
					node[p->node_num].visited = 1;
					node_to_reconnect.push(p->node_num);
				}

				else
				{
					//如果遇到可能导致再次深度优先搜索的结点
					if (node[p->node_num].out_degree > 1)
					{
						node[p->node_num].visited = 1;
						node_to_reconnect.push(p->node_num);
						node_to_reconsider.push(p->node_num);
					}

					//如果遇到可移除的结点
					else
					{
						node_to_remove.push(p->node_num);
						depth_first_search(p->node_num);
					}
				}
			}
		}
		p = p->next;
	}
	return;
}


//重新连接结点
void reconnect_nodes(int node_num)
{
	Linked_List *p;
	while (node_to_reconnect.empty() == 0)
	{
		if (adjacency_matrix_forward[node_num][node_to_reconnect.top()] == 0)
		{
			connect_nodes(node_to_reconnect.top(), node_num);
		}
		node_to_reconnect.pop();
	}
}


//移除结点
void remove_node()
{
	int i;
	Linked_List *p;
	int node_num;
	int node_temp;
	while (node_to_remove.empty() == 0)
	{
		node_num = node_to_remove.top();

		//在上结点所在链表中移除该结点
		node_temp = adjacency_list_reverse[node_num]->node_num;
		adjacency_list_forward[node_temp] = remove_node_from_list(adjacency_list_forward[node_temp], node_num);
		node[node_temp].in_degree--;

		//在下结点所在链表中移除该结点
		p = adjacency_list_forward[node_num];
		while (p != NULL)
		{
			node_temp = p->node_num;
			adjacency_list_reverse[node_temp] = remove_node_from_list(adjacency_list_reverse[node_temp], node_num);
			node[node_temp].out_degree--;
			p = p->next;
		}

		//销毁该结点所在链表
		adjacency_list_forward[node_num] = destroy_list(adjacency_list_forward[node_num]);
		adjacency_list_reverse[node_num] = destroy_list(adjacency_list_reverse[node_num]);
		node[node_num].in_degree = 0;
		node[node_num].out_degree = 0;

		//更新adjacency_matrix_forward
		for (i = 0; i < N_process + N_function; i++)
		{
			adjacency_matrix_forward[i][node_num] = 0;
			adjacency_matrix_forward[node_num][i] = 0;
		}

		//更新removed
		process[node_num].removed = 1;

		node_to_remove.pop();
	}
}


//判断是否需要再次深度优先搜索
bool continue_search()
{
	while (node_to_reconsider.empty() == 0)
	{
		if (node[node_to_reconsider.top()].out_degree == 1)
		{
			return 1;
		}
		node_to_reconsider.pop();
	}
	return 0;
}


//化简网络
void simplify_network()
{
	int i;
	for (i = 0; i < N_process; i++)
	{
		if (process[i].removed == 0 && (process[i].logic_gate == 1 || process[i].logic_gate == 2))
		{
			refresh_visited();
			depth_first_search(i);
			reconnect_nodes(i);
			remove_node();
			while (continue_search() == 1)
			{
				refresh_visited();
				depth_first_search(i);
				reconnect_nodes(i);
				remove_node();
			}
		}
	}
}


/**************************************************确定遍历过程结点的顺序**************************************************/


//确定遍历过程结点的顺序
void find_sequence_process()
{
	int i;
	Linked_List *p;
	int in_degree_temp[MAXN_process];
	int visited_count = 0;

	refresh_visited();

	//初始化in_degree_temp，确定N_process_exist
	N_process_exist = N_process;
	for (i = 0; i < N_process; i++)
	{
		in_degree_temp[i] = node[i].in_degree;
		if (process[i].removed == 1)
		{
			N_process_exist--;
		}
	}

	//遍历功能结点，更新in_degree_temp
	for (i = N_process; i < N_process + N_function; i++)
	{
		p = adjacency_list_reverse[i];
		while (p != NULL)
		{
			in_degree_temp[p->node_num]--;
			p = p->next;
		}
	}

	//确定遍历过程结点的顺序
	while (visited_count < N_process_exist)
	{
		for (i = 0; i < N_process; i++)
		{
			if (process[i].removed == 0 && node[i].visited == 0 && in_degree_temp[i] == 0)
			{
				sequence_process[visited_count] = i;
				visited_count++;
				node[i].visited = 1;
				p = adjacency_list_reverse[i];
				while (p != NULL)
				{
					in_degree_temp[p->node_num]--;
					p = p->next;
				}
			}
		}
	}
}


/**************************************************测试**************************************************/


//在每次测试前更新参数
void refresh_parameter()
{
	int i, j;

	//更新机器结点
	for (i = 0; i < N_machine; i++)
	{
		machine[i].reliability_move_step = 0;
		machine[i].capacity = machine[i].initial_capacity;
		machine[i].fault = 0;
		machine[i].backup_recover_time_remaining = 0;
	}

	//更新零件结点
	for (i = 0; i < N_component; i++)
	{
		for (j = 0; j < backup_place_sum; j++)
		{
			component[i].backup_available_sum[j] = component[i].backup_initial_sum[j];
			while (component[i].backup_arrive_time_remaining[j].empty() == 0)
			{
				component[i].backup_arrive_time_remaining[j].pop();
			}
		}
		while (component[i].repair_recover_time_remaining.empty() == 0)
		{
			component[i].repair_recover_time_remaining.pop();
		}
		while (component[i].repair_arrive_time_remaining.empty() == 0)
		{
			component[i].repair_arrive_time_remaining.pop();
		}
	}

	//更新资源结点
	for (i = 0; i < N_resource; i++)
	{
		for (j = 0; j < repair_place_sum; j++)
		{
			resource[i].backup_available_sum[j] = resource[i].backup_initial_sum[j];
		}
	}

	//更新等待修复的零件
	for (i = 0; i < repair_place_sum; i++)
	{
		while (component_waiting_for_repair[i].empty() == 0)
		{
			component_waiting_for_repair[i].pop();
		}
	}
}


//生成随机数
double generate_probability()
{
	int i;
	double probability = 0;
	int precision_level = 3;        //精度等级
	double denominator = 1;
	for (i = 0; i < precision_level; i++)
	{
		denominator *= 1000;
		probability += rand() % 1000 * 1.0 / denominator;
	}
	probability += 1.0 / denominator;
	return probability;
}


//查看是否有足够的资源来修理零件
void repair_component(int component_num)
{
	bool flag = 1;
	Linked_List *p;

	//查看资源是否充足
	p = component[component_num].resource;
	while (p != NULL&&flag == 1)
	{
		if (resource[p->node_num].backup_available_sum[component[component_num].repair_place] < p->node_sum)
		{
			flag = 0;
		}
		p = p->next;
	}

	//如果资源充足
	if (flag == 1)
	{
		p = component[component_num].resource;
		while (p != NULL)
		{
			resource[p->node_num].backup_available_sum[component[component_num].repair_place] -= p->node_sum;
			result.cost += resource[p->node_num].repair_cost_per_time_unit*p->node_sum*component[component_num].repair_time;
			p = p->next;
		}
		component[component_num].repair_recover_time_remaining.push(component[component_num].repair_time);
	}

	//如果资源不足
	else
	{
		component_waiting_for_repair[component[component_num].repair_place].push(component_num);
	}
}


//查看是否有足够的零件来更换机器
void change_machine(int machine_num, int time_now)
{
	Linked_List *p;
	bool flag = 1;

	//查看零件是否充足
	p = machine[machine_num].component;
	while (p != NULL&&flag == 1)
	{
		if (component[p->node_num].backup_available_sum[0] < p->node_sum)
		{
			flag = 0;
		}
		p = p->next;
	}

	//如果零件充足
	if (flag == 1)
	{
		p = machine[machine_num].component;
		while (p != NULL)
		{
			component[p->node_num].backup_available_sum[0] -= p->node_sum;
			p = p->next;
		}
		machine[machine_num].reliability_move_step = time_now;
		machine[machine_num].capacity = machine[machine_num].initial_capacity*machine[machine_num].backup_capacity_ratio;
		machine[machine_num].fault = 0;
		machine[machine_num].backup_recover_time_remaining = machine[machine_num].backup_delay;
	}
}


//修复机器
void repair_machine(int machine_num)
{
	int i;
	Linked_List *p;

	p = machine[machine_num].component;
	while (p != NULL)
	{

		//如果修复地点在基层
		if (component[p->node_num].repair_place == 0)
		{
			for (i = 0; i < p->node_sum; i++)
			{
				repair_component(p->node_num);
			}
		}

		//如果修复地点在中继
		else
		{
			for (i = 0; i < p->node_sum; i++)
			{
				component[p->node_num].repair_arrive_time_remaining.push(transport.time[0][component[p->node_num].repair_place]);
			}

			result.cost += transport.cost*component[p->node_num].weight* transport.time[0][component[p->node_num].repair_place] * p->node_sum;
		}
		p = p->next;
	}
}


//判断各个过程结点是否故障
void judge_fault()
{
	int i;
	Linked_List *p;
	bool flag;
	int node_num;
	for (i = 0; i < N_process_exist; i++)
	{
		node_num = sequence_process[i];
		p = adjacency_list_forward[node_num];

		//与门
		if (process[node_num].logic_gate == 1)
		{
			flag = 1;
			node[node_num].fault = 1;
			while (flag == 1 && p != NULL)
			{
				if (node[p->node_num].fault == 0)
				{
					node[node_num].fault = 0;
					flag = 0;
				}
				p = p->next;
			}
		}

		//或门
		else if (process[node_num].logic_gate == 2)
		{
			flag = 1;
			node[node_num].fault = 0;
			while (flag == 1 && p != NULL)
			{
				if (node[p->node_num].fault == 1)
				{
					node[node_num].fault = 1;
					flag = 0;
				}
				p = p->next;
			}
		}

		//表决门
		else if (process[node_num].logic_gate == 3)
		{
			flag = 1;
			node[node_num].fault = 0;
			int sum_temp = 0;
			while (flag == 1 && p != NULL)
			{
				if (node[p->node_num].fault == 1)
				{
					sum_temp++;
				}
				if (sum_temp >= process[node_num].least_event)
				{
					node[node_num].fault = 1;
					flag = 0;
				}
				p = p->next;
			}
		}

		//异或门
		else if (process[node_num].logic_gate == 4)
		{
			if (node[p->node_num].fault == node[p->next->node_num].fault)
			{
				node[node_num].fault = 1;
			}
			else
			{
				node[node_num].fault = 0;
			}
		}

		//同或门
		else if (process[node_num].logic_gate == 5)
		{
			if (node[p->node_num].fault == node[p->next->node_num].fault)
			{
				node[node_num].fault = 0;
			}
			else
			{
				node[node_num].fault = 1;
			}
		}
	}
}


//测试
void test()
{
	int i, j, k, l;
	Linked_List *p;
	bool flag;
	int len_temp, node_temp;
	double probability;
	double system_capacity;        //系统的能力

	for (i = 0; i < time_sum; i++)
	{
//		printf("  time: %d\n", i);
		system_capacity = 1;

		//查看机器结点是否需要预防性修复_
		if (i%preventive_repair_time == 0)
		{
			for (j = 0; j < N_machine; j++)
			{
				if (machine[j].reliability[i - machine[j].reliability_move_step] < machine[j].repair_reliability)
				{
					machine[j].fault = 1;
					change_machine(j, i);
					repair_machine(j);
				}
			}
		}

		//遍历功能结点
		for (j = 0; j < N_function; j++)
		{
			function[j].capacity = 0;
			flag = 1;
			p = function[j].machine;
			while (p != NULL&&flag == 1)
			{
				//如果机器结点发生故障，查看当前是否可以进行更换
				if (machine[p->node_num].fault == 1)
				{
					change_machine(p->node_num, i);
				}

				//如果机器结点未发生故障
				if (machine[p->node_num].fault == 0)
				{
					//生成随机数，确定机器结点的状态
					probability = generate_probability();
					if (probability > machine[p->node_num].reliability[i - machine[p->node_num].reliability_move_step])
					{
						machine[p->node_num].fault = 1;
						change_machine(p->node_num, i);

						probability = generate_probability();
						if (probability > machine[p->node_num].repair_unrecoverable_rate)
						{
							repair_machine(p->node_num);
						}
					}

					if (machine[p->node_num].fault == 0)
					{
						function[j].capacity += machine[p->node_num].capacity;
						machine[p->node_num].used = 1;
					}

					else
					{
						machine[p->node_num].used = 0;
					}

					//如果功能结点的能力达到1
					if (function[j].capacity >= 1)
					{
						flag = 0;
					}
				}

				//如果机器结点能力为0
				else
				{
					machine[p->node_num].used = 0;
				}

				p = p->next;
			}

			//更新闲置机器的状态
			while (p != NULL)
			{
				machine[p->node_num].used = 0;
				p = p->next;
			}

			//如果功能结点故障
			if (function[j].capacity == 0)
			{
				node[j + N_process].fault = 1;
			}

			//如果功能结点未故障
			else
			{
				node[j + N_process].fault = 0;
			}
		}

		//判断系统是否可以正常工作
		judge_fault();
		for (j = 0; j < N_process_exist; j++)
		{
			result.fault[sequence_process[j]][i] += node[sequence_process[j]].fault;
		}

		//如果系统可以正常工作
		if (node[sequence_process[N_process_exist]].fault == 0)
		{
			//查看是否有能力未达到1的功能结点
			for (j = 0; j < N_function; j++)
			{
				if (function[j].capacity < system_capacity)
				{
					//先将该功能结点设为故障，判断系统是否可以正常工作
					node[j].fault = 1;
					judge_fault();

					//如果该功能结点不可或缺
					if (node[sequence_process[N_process_exist]].fault == 1)
					{
						system_capacity = function[j].capacity;
					}

					//将功能结点改为正常
					node[j].fault = 0;
				}
			}

			//更新机器结点参数
			for (j = 0; j < N_machine; j++)
			{
				if (machine[j].used == 1)
				{
					if (machine[j].backup_recover_time_remaining > 0)
					{
						machine[j].backup_recover_time_remaining--;

						//如果恢复正常，更新能力
						if (machine[j].backup_recover_time_remaining == 0)
						{
							machine[j].capacity = machine[j].initial_capacity;
						}
					}
				}
				else
				{
					machine[j].reliability_move_step++;
				}
			}
		}

		//如果系统发生故障
		else
		{
			system_capacity = 0;

			for (j = 0; j < N_machine; j++)
			{
				machine[j].reliability_move_step++;
			}
		}
		result.capacity[i] += system_capacity;

		//遍历零件结点
		for (j = 0; j < N_component; j++)
		{
			//遍历基层和中继
			for (k = 0; k < backup_place_sum - 1; k++)
			{
				//查看是否需要向基层和中继运输零件备份
				if (component[j].backup_available_sum[k] < component[j].backup_minimum_sum[k])
				{
					if (component[j].backup_available_sum[k + 1] >= component[j].backup_minimum_sum[k] - component[j].backup_available_sum[k])
					{
						len_temp = component[j].backup_minimum_sum[k] - component[j].backup_available_sum[k];
					}
					else
					{
						len_temp = component[j].backup_available_sum[k + 1];
					}
					component[j].backup_available_sum[k + 1] -= len_temp;
					for (l = 0; l < len_temp; l++)
					{
						component[j].backup_arrive_time_remaining[k].push(transport.time[k + 1][k]);
					}
					result.cost += len_temp*transport.cost*component[j].weight*transport.time[k + 1][k];
				}

				//更新备份距离送达的时间
				len_temp = component[j].backup_arrive_time_remaining[k].size();
				for (l = 0; l < len_temp; l++)
				{
					node_temp = component[j].backup_arrive_time_remaining[k].front() - 1;
					component[j].backup_arrive_time_remaining[k].pop();

					//如果备份送达
					if (node_temp == 0)
					{
						component[j].backup_available_sum[k]++;
					}

					//如果备份未送达
					else
					{
						component[j].backup_arrive_time_remaining[k].push(node_temp);
					}
				}
			}

			//计算储存费用
			for (k = 0; k < backup_place_sum; k++)
			{
				result.cost += component[j].backup_available_sum[k] * component[j].storage_cost_per_volume[k] * component[j].volume;
			}

			//更新损坏的零件距离修好的时间
			len_temp = component[j].repair_recover_time_remaining.size();
			for (k = 0; k < len_temp; k++)
			{
				node_temp = component[j].repair_recover_time_remaining.front() - 1;
				component[j].repair_recover_time_remaining.pop();

				//如果修好了
				if (node_temp == 0)
				{
					component[j].backup_available_sum[component[j].repair_place]++;

					//释放资源
					p = component[j].resource;
					while (p != NULL)
					{
						resource[p->node_num].backup_available_sum[component[j].repair_place] += p->node_sum;
						p = p->next;
					}
				}

				//如果未修好
				else
				{
					component[j].repair_recover_time_remaining.push(node_temp);
				}
			}

			//更新损坏的零件距离送达修复地点的时间
			len_temp = component[j].repair_arrive_time_remaining.size();
			for (k = 0; k < len_temp; k++)
			{
				node_temp = component[j].repair_arrive_time_remaining.front() - 1;
				component[j].repair_arrive_time_remaining.pop();

				//如果送达了
				if (node_temp == 0)
				{
					component_waiting_for_repair[component[j].repair_place].push(j);
				}

				//如果未送达
				else
				{
					component[j].repair_arrive_time_remaining.push(node_temp);
				}
			}
		}

		//更新等待修复的零件
		for (j = 0; j < repair_place_sum; j++)
		{
			len_temp = component_waiting_for_repair[j].size();
			for (k = 0; k < len_temp; k++)
			{
				node_temp = component_waiting_for_repair[j].front();
				component_waiting_for_repair[j].pop();
				repair_component(node_temp);
			}
		}
	}
}


/**************************************************输出参数**************************************************/


//输出结果
void fprint_result()
{
	int i, j, k;
	FILE *fp1 = fopen("output result availability.txt", "w");
	FILE *fp2 = fopen("output result reliability.txt", "w");
	FILE *fp3 = fopen("output result capacity.txt", "w");
	FILE *fp4 = fopen("output result cost.txt", "w");
	int fault_count_temp;

	for (i = 0; i < time_sum; i++)
	{
		fprintf(fp1, "%10d", i);
		fprintf(fp2, "%10d", i);
		fprintf(fp3, "%10d", i);
		for (j = 0; j < N_process; j++)
		{
			fault_count_temp = 0;
			for (k = 0; k <= i; k++)
			{
				fault_count_temp += result.fault[j][k];
			}
			fprintf(fp1, "%10.6lf", 1 - fault_count_temp*1.0 / test_sum / (i + 1));
			fprintf(fp2, "%10.6lf", 1 - result.fault[j][i] * 1.0 / test_sum);
		}
		fprintf(fp3, "%10.6lf\n", result.capacity[i] / test_sum);
		fprintf(fp1, "\n");
		fprintf(fp2, "\n");
	}

	fprintf(fp4, "%lf", result.cost*1.0 / test_sum);

	fclose(fp1);
	fclose(fp2);
	fclose(fp3);
	fclose(fp4);
}


int main()
{
	fscan_N();
	fscan_network();
	fscan_process();
	fscan_function();
	fscan_standard_normal_distribution_table();
	fscan_machine();
	fscan_component();
	fscan_resource();
	fscan_transport();
	simplify_network();
	find_sequence_process();
	srand((unsigned)time(NULL));
	for (int i = 0; i < test_sum; i++)
	{
		printf("test num: %d\n", i + 1);
		refresh_parameter();
		test();
	}
	fprint_result();
	return 0;
}

