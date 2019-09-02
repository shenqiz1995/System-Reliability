// black-box-testing.cpp : �������̨Ӧ�ó������ڵ㡣
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
#define MAXN_process 100        //���̽��������
#define MAXN_function 100        //���ܽ��������
#define MAXN_machine 100        //�������������
#define MAXN_component 100        //������������
#define MAXN_resource 100        //��Դ���������
#define MAXN_row 45        //��׼��̬�ֲ�������
#define MAXN_column 10        //��׼��̬�ֲ�������
#define MAXN_time 100        //����ʱ������
#define MAXN_backup_place 3        //�洢�ص�������
#define MAXN_repair_place 2        //�޸��ص�������


typedef struct linked_list
{
	int node_num;        //�����
	int node_sum;        //�����Ŀ
	struct linked_list *next;        //ָ��
} Linked_List;
Linked_List *adjacency_list_forward[MAXN_process + MAXN_function];        //�Զ����µ��ڽӱ�
Linked_List *adjacency_list_reverse[MAXN_process + MAXN_function];        //�Ե����ϵ��ڽӱ�
bool adjacency_matrix_forward[MAXN_process + MAXN_function][MAXN_process + MAXN_function];        //�Զ����µ��ڽӾ���

struct
{
	int in_degree;        //���
	int out_degree;        //����
	bool fault;        //��ǰ״̬��0Ϊû�й��ϣ�1Ϊ�������ϣ�
	bool visited;        //�Ƿ��Ѿ����ʹ���0Ϊδ���ʹ���1Ϊ���ʹ���
} node[MAXN_process + MAXN_function];

struct
{
	bool removable;        //�Ƿ�����Ƴ���0��ʾ�����Ƴ���1��ʾ�����Ƴ���
	bool removed;        //�Ƿ��Ѿ����Ƴ���0��ʾ��δ���Ƴ���1��ʾ�Ѿ����Ƴ���
	int logic_gate;        //�߼��ţ�1Ϊ���ţ�2Ϊ���ţ�3Ϊ����ţ�4Ϊ����ţ�5Ϊͬ���ţ�
	int least_event;        //��������ٷ������¼���
} process[MAXN_process];

struct
{
	double capacity;        //����
	Linked_List *machine;        //����Ļ���
} function[MAXN_function];

struct
{
	//�ɿ�������
	double reliability[MAXN_time];        //��ʼ�Ŀɿ���
	int reliability_move_step;        //�ɿ�����������ڳ�ʼ�Ŀɿ���������Ҫ�ƶ��Ĳ���

	//��������
	double initial_capacity;        //��ʼ����
	double capacity;        //����
	bool fault;        //��ǰ״̬��0Ϊû�й��ϣ�1Ϊ�������ϣ�

	//��������
	int backup_initial_sum[MAXN_backup_place];        //��ʼ�ı�����Ŀ����������ǰ���ڹ����Ļ�����
	int backup_delay;        //�����ӳ�ʱ�䣨�ȱ����ӳ�ʱ��Ϊ0���±��ݺ��䱸���ӳ�ʱ�����0��
	double backup_capacity_ratio;        //�����ӳ�ʱ���ڵĻ����������������±��ݱ�������0���䱸�ݱ���Ϊ0��
	int backup_recover_time_remaining;        //��ǰ���ڹ����Ļ��������������е�ʱ��

	//�޸�����
	double repair_reliability;        //�����޸�ʱ�Ŀɿ��ȣ�ά�����޸��ɿ���Ϊ0��Ԥ�����޸��ɿ��ȴ���0��
	double repair_unrecoverable_rate;        //������

	//�������
	Linked_List *component;        //��������

	//ʹ������
	bool used;        //�Ƿ�ʹ��
} machine[MAXN_machine];

struct
{
	//��������
	int backup_initial_sum[MAXN_backup_place];        //��ʼ�ı�����Ŀ����������ǰ���ڹ����������
	int backup_available_sum[MAXN_backup_place];        //����ʹ�õı�����Ŀ����������ǰ���ڹ����������
	int backup_minimum_sum[MAXN_backup_place - 1];        //���ٵı�����Ŀ
	queue<int> backup_arrive_time_remaining[MAXN_backup_place];        //���ݾ����ʹ��ʱ��

	//�޸�����
	int repair_time;        //�޸�ʱ��
	int repair_place;        //�޸��ص㣨0Ϊ�ڻ����޸���1Ϊ���м��޸���
	queue<int> repair_recover_time_remaining;        //�𻵵���������޸��õ�ʱ��
	queue<int> repair_arrive_time_remaining;        //�𻵵���������ʹ��޸��ص��ʱ��

	//��Դ����
	Linked_List *resource;        //�������Դ

	//�洢����������
	double volume;        //���
	double weight;        //����
	double storage_cost_per_volume[MAXN_backup_place];        //���ݵ�λ����Ĵ������
} component[MAXN_component];

struct
{
	//��������
	int backup_initial_sum[MAXN_repair_place];        //��ʼ�ı�����Ŀ
	int backup_available_sum[MAXN_repair_place];        //����ʹ�õı�����Ŀ

	//�޸�����
	double repair_cost_per_time_unit;        //��ʱ����
} resource[MAXN_resource];

struct
{
	int time[MAXN_backup_place][MAXN_backup_place];        //����ʱ��
	double cost;        //�������
} transport;

struct
{
	int fault[MAXN_process][MAXN_time];        //���ϴ���
	double capacity[MAXN_time];        //����
	double cost;        //����
} result;

int N_process;        //���̽����
int N_process_exist;        //δ���Ƴ��Ĺ��̽����
int N_function;        //���ܽ����
int N_machine;        //���������
int N_component;        //��������
int N_resource;        //��Դ�����
double standard_normal_distribution_table[MAXN_row][MAXN_column];        //��׼��̫�ֲ���
int time_sum = MAXN_time;
int test_sum = 100;
int preventive_repair_time = 8;        //����Ԥ�����޸��ļ��
int backup_place_sum = MAXN_backup_place;        //�洢�ص���
int repair_place_sum = MAXN_repair_place;        //�޸��ص���
stack<int> node_to_remove;        //��һ��������������н��ᱻ�Ƴ��Ľ��
stack<int> node_to_reconnect;        //��һ��������������н��ᱻ�������ӵĽ��
stack<int> node_to_reconsider;        //��һ��������������п��ܵ����ٴ�������������Ľ��
int sequence_process[MAXN_process];        //�������̽���˳��
queue<int> component_waiting_for_repair[MAXN_repair_place];        //�ȴ��޸������


/**************************************************������Ļ�������**************************************************/


//������ͷ����һ�����
Linked_List *insert_node_to_list(Linked_List *list, int node_num, int node_sum)
{
	Linked_List *p;
	p = (Linked_List*)malloc(sizeof(Linked_List));
	p->node_num = node_num;
	p->node_sum = node_sum;
	p->next = list;
	return p;
}


//���������Ƴ�һ�����
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


//��������
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


/**************************************************�������**************************************************/


//���ӽ��
void connect_nodes(int input, int output)
{
	adjacency_list_forward[output] = insert_node_to_list(adjacency_list_forward[output], input, 1);
	adjacency_list_reverse[input] = insert_node_to_list(adjacency_list_reverse[input], output, 1);
	adjacency_matrix_forward[output][input] = 1;
	node[output].in_degree++;
	node[input].out_degree++;
}


//��ȡ�����
void fscan_N()
{
	FILE *fp = fopen("input N.txt", "r");
	fscanf(fp, "%d%d%d%d%d", &N_process, &N_function, &N_machine, &N_component, &N_resource);
	fclose(fp);
}


//��ȡ����
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


//������̽��
void fscan_process()
{
	int i;
	FILE *fp = fopen("input process.txt", "r");
	int node_num;
	for (i = 0; i < N_process; i++)
	{
		fscanf(fp, "%d", &node_num);
		fscanf(fp, "%d%d", &process[node_num].removable, &process[node_num].logic_gate);

		//����߼����Ǳ����
		if (process[node_num].logic_gate == 3)
		{
			fscanf(fp, "%d", &process[node_num].least_event);
		}
	}
	fclose(fp);
}


//���빦�ܽ��
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


//�����׼��̬�ֲ���
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


//�����׼��̬�ֲ�����ֵ
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

	//��Ϊ����
	if (negative == 1)
	{
		result = 1 - result;
	}

	return result;
}


//����������
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
	int distribution_type;        //�ֲ���1Ϊָ���ֲ���2Ϊ��̬�ֲ���3Ϊ������̬�ֲ���4Ϊ�������ֲ���
	double theta, mu, sigma, eta, m;
	int component_type_sum, component_num, component_sum;

	//����ɿ�������
	for (i = 0; i < N_machine; i++)
	{
		fscanf(fp1, "%d%d", &node_num, &distribution_type);

		//ָ���ֲ�
		if (distribution_type == 1)
		{
			fscanf(fp1, "%lf", &theta);
			for (j = 0; j < time_sum; j++)
			{
				machine[node_num].reliability[j] = exp(-j*1.0 / theta);
			}
		}

		//��̬�ֲ�
		else if (distribution_type == 2)
		{
			fscanf(fp1, "%lf%lf", &mu, &sigma);
			for (j = 0; j < time_sum; j++)
			{
				machine[node_num].reliability[j] = 1 - standard_normal_distribution_function((j*1.0 - mu) / sigma);
			}
		}

		//������̬�ֲ�
		else if (distribution_type == 3)
		{
			fscanf(fp1, "%lf%lf", &mu, &sigma);
			for (j = 0; j < time_sum; j++)
			{
				machine[node_num].reliability[j] = 1 - standard_normal_distribution_function((log(j*1.0) - mu) / sigma);
			}
		}

		//�������ֲ�
		else if (distribution_type == 4)
		{
			fscanf(fp1, "%lf%lf", &eta, &m);
			for (j = 0; j < time_sum; j++)
			{
				machine[node_num].reliability[j] = exp(-pow(j*1.0 / eta, m));
			}
		}
	}

	//������������
	for (i = 0; i < N_machine; i++)
	{
		fscanf(fp2, "%d", &node_num);
		fscanf(fp2, "%lf", &machine[node_num].initial_capacity);
	}

	//�����������
	for (i = 0; i < N_machine; i++)
	{
		fscanf(fp5, "%d%d", &node_num, &component_type_sum);
		for (j = 0; j < component_type_sum; j++)
		{
			fscanf(fp5, "%d%d", &component_num, &component_sum);
			machine[i].component = insert_node_to_list(machine[i].component, component_num, component_sum);
		}
	}

	//���뱸������
	for (i = 0; i < N_machine; i++)
	{
		fscanf(fp3, "%d", &node_num);
		for (j = 0; j < backup_place_sum; j++)
		{
			fscanf(fp3, "%d", &machine[node_num].backup_initial_sum[j]);

			//��������ı�����
			p = machine[node_num].component;
			while (p != NULL)
			{
				component[p->node_num].backup_initial_sum[j] += machine[node_num].backup_initial_sum[j];
				p = p->next;
			}
		}
		fscanf(fp3, "%d%lf", &machine[node_num].backup_delay, &machine[node_num].backup_capacity_ratio);
	}

	//�����޸�����
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


//����������
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

	//���뱸������
	for (i = 0; i < N_component; i++)
	{
		fscanf(fp1, "%d", &node_num);
		for (j = 0; j < backup_place_sum - 1; j++)
		{
			fscanf(fp1, "%lf", &backup_ratio);
			component[i].backup_minimum_sum[j] = int(component[i].backup_initial_sum[j] * backup_ratio);
		}
	}

	//�����޸�����
	for (i = 0; i < N_component; i++)
	{
		fscanf(fp2, "%d", &node_num);
		fscanf(fp2, "%d%d", &component[node_num].repair_time, &component[node_num].repair_place);
	}

	//������Դ����
	for (i = 0; i < N_component; i++)
	{
		fscanf(fp3, "%d%d", &node_num, &resource_type_sum);
		for (j = 0; j < resource_type_sum; j++)
		{
			fscanf(fp3, "%d%d", &resource_num, &resource_sum);
			component[i].resource = insert_node_to_list(component[i].resource, resource_num, resource_sum);
		}
	}

	//����洢����������
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


//������Դ���
void fscan_resource()
{
	int i, j;
	FILE *fp1 = fopen("input resource backup.txt", "r");
	FILE *fp2 = fopen("input resource repair.txt", "r");
	int node_num;

	//���뱸������
	for (i = 0; i < N_resource; i++)
	{
		fscanf(fp1, "%d", &node_num);
		for (j = 0; j < repair_place_sum; j++)
		{
			fscanf(fp1, "%d", &resource[node_num].backup_initial_sum[j]);
		}
	}

	//�����޸�����
	for (i = 0; i < N_resource; i++)
	{
		fscanf(fp2, "%d", &node_num);
		fscanf(fp2, "%lf", &resource[node_num].repair_cost_per_time_unit);
	}

	fclose(fp1);
	fclose(fp2);
}


//�����������
void fscan_transport()
{
	int i, j, k;
	FILE *fp = fopen("input transport.txt", "r");

	//�������������洢�ص�������ʱ��
	for (i = 0; i < backup_place_sum - 1; i++)
	{
		fscanf(fp, "%d", &transport.time[i][i + 1]);
		transport.time[i + 1][i] = transport.time[i][i + 1];
	}

	//��������ʱ�����
	for (i = 0; i < backup_place_sum - 2; i++)
	{
		for (j = 0; j < backup_place_sum - i - 2; j++)
		{
			transport.time[j][j + i + 2] = transport.time[j][j + i + 1] + transport.time[j + i + 1][j + i + 2];
			transport.time[j + i + 2][j] = transport.time[j][j + i + 2];
		}
	}

	//���뻨��
	fscanf(fp, "%lf", &transport.cost);

	fclose(fp);
}


/**************************************************��������**************************************************/


//����visited
void refresh_visited()
{
	int i;
	for (i = 0; i < N_process + N_function; i++)
	{
		node[i].visited = 0;
	}
}


//�����������
void depth_first_search(int node_num)
{
	Linked_List *p;
	node[node_num].visited = 1;
	p = adjacency_list_forward[node_num];
	while (p != NULL)
	{
		if (node[p->node_num].visited == 0)
		{
			//����������ܽ��
			if (p->node_num >= N_process)
			{
				node[p->node_num].visited = 1;
				node_to_reconnect.push(p->node_num);
			}

			//����������̽��
			else
			{
				//��������߼��Ų�һ�»��ǲ����Ƴ��Ľ��
				if (process[p->node_num].logic_gate != process[node_num].logic_gate || process[p->node_num].removable == 0)
				{
					node[p->node_num].visited = 1;
					node_to_reconnect.push(p->node_num);
				}

				else
				{
					//����������ܵ����ٴ�������������Ľ��
					if (node[p->node_num].out_degree > 1)
					{
						node[p->node_num].visited = 1;
						node_to_reconnect.push(p->node_num);
						node_to_reconsider.push(p->node_num);
					}

					//����������Ƴ��Ľ��
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


//�������ӽ��
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


//�Ƴ����
void remove_node()
{
	int i;
	Linked_List *p;
	int node_num;
	int node_temp;
	while (node_to_remove.empty() == 0)
	{
		node_num = node_to_remove.top();

		//���Ͻ�������������Ƴ��ý��
		node_temp = adjacency_list_reverse[node_num]->node_num;
		adjacency_list_forward[node_temp] = remove_node_from_list(adjacency_list_forward[node_temp], node_num);
		node[node_temp].in_degree--;

		//���½�������������Ƴ��ý��
		p = adjacency_list_forward[node_num];
		while (p != NULL)
		{
			node_temp = p->node_num;
			adjacency_list_reverse[node_temp] = remove_node_from_list(adjacency_list_reverse[node_temp], node_num);
			node[node_temp].out_degree--;
			p = p->next;
		}

		//���ٸý����������
		adjacency_list_forward[node_num] = destroy_list(adjacency_list_forward[node_num]);
		adjacency_list_reverse[node_num] = destroy_list(adjacency_list_reverse[node_num]);
		node[node_num].in_degree = 0;
		node[node_num].out_degree = 0;

		//����adjacency_matrix_forward
		for (i = 0; i < N_process + N_function; i++)
		{
			adjacency_matrix_forward[i][node_num] = 0;
			adjacency_matrix_forward[node_num][i] = 0;
		}

		//����removed
		process[node_num].removed = 1;

		node_to_remove.pop();
	}
}


//�ж��Ƿ���Ҫ�ٴ������������
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


//��������
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


/**************************************************ȷ���������̽���˳��**************************************************/


//ȷ���������̽���˳��
void find_sequence_process()
{
	int i;
	Linked_List *p;
	int in_degree_temp[MAXN_process];
	int visited_count = 0;

	refresh_visited();

	//��ʼ��in_degree_temp��ȷ��N_process_exist
	N_process_exist = N_process;
	for (i = 0; i < N_process; i++)
	{
		in_degree_temp[i] = node[i].in_degree;
		if (process[i].removed == 1)
		{
			N_process_exist--;
		}
	}

	//�������ܽ�㣬����in_degree_temp
	for (i = N_process; i < N_process + N_function; i++)
	{
		p = adjacency_list_reverse[i];
		while (p != NULL)
		{
			in_degree_temp[p->node_num]--;
			p = p->next;
		}
	}

	//ȷ���������̽���˳��
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


/**************************************************����**************************************************/


//��ÿ�β���ǰ���²���
void refresh_parameter()
{
	int i, j;

	//���»������
	for (i = 0; i < N_machine; i++)
	{
		machine[i].reliability_move_step = 0;
		machine[i].capacity = machine[i].initial_capacity;
		machine[i].fault = 0;
		machine[i].backup_recover_time_remaining = 0;
	}

	//����������
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

	//������Դ���
	for (i = 0; i < N_resource; i++)
	{
		for (j = 0; j < repair_place_sum; j++)
		{
			resource[i].backup_available_sum[j] = resource[i].backup_initial_sum[j];
		}
	}

	//���µȴ��޸������
	for (i = 0; i < repair_place_sum; i++)
	{
		while (component_waiting_for_repair[i].empty() == 0)
		{
			component_waiting_for_repair[i].pop();
		}
	}
}


//���������
double generate_probability()
{
	int i;
	double probability = 0;
	int precision_level = 3;        //���ȵȼ�
	double denominator = 1;
	for (i = 0; i < precision_level; i++)
	{
		denominator *= 1000;
		probability += rand() % 1000 * 1.0 / denominator;
	}
	probability += 1.0 / denominator;
	return probability;
}


//�鿴�Ƿ����㹻����Դ���������
void repair_component(int component_num)
{
	bool flag = 1;
	Linked_List *p;

	//�鿴��Դ�Ƿ����
	p = component[component_num].resource;
	while (p != NULL&&flag == 1)
	{
		if (resource[p->node_num].backup_available_sum[component[component_num].repair_place] < p->node_sum)
		{
			flag = 0;
		}
		p = p->next;
	}

	//�����Դ����
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

	//�����Դ����
	else
	{
		component_waiting_for_repair[component[component_num].repair_place].push(component_num);
	}
}


//�鿴�Ƿ����㹻���������������
void change_machine(int machine_num, int time_now)
{
	Linked_List *p;
	bool flag = 1;

	//�鿴����Ƿ����
	p = machine[machine_num].component;
	while (p != NULL&&flag == 1)
	{
		if (component[p->node_num].backup_available_sum[0] < p->node_sum)
		{
			flag = 0;
		}
		p = p->next;
	}

	//����������
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


//�޸�����
void repair_machine(int machine_num)
{
	int i;
	Linked_List *p;

	p = machine[machine_num].component;
	while (p != NULL)
	{

		//����޸��ص��ڻ���
		if (component[p->node_num].repair_place == 0)
		{
			for (i = 0; i < p->node_sum; i++)
			{
				repair_component(p->node_num);
			}
		}

		//����޸��ص����м�
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


//�жϸ������̽���Ƿ����
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

		//����
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

		//����
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

		//�����
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

		//�����
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

		//ͬ����
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


//����
void test()
{
	int i, j, k, l;
	Linked_List *p;
	bool flag;
	int len_temp, node_temp;
	double probability;
	double system_capacity;        //ϵͳ������

	for (i = 0; i < time_sum; i++)
	{
//		printf("  time: %d\n", i);
		system_capacity = 1;

		//�鿴��������Ƿ���ҪԤ�����޸�_
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

		//�������ܽ��
		for (j = 0; j < N_function; j++)
		{
			function[j].capacity = 0;
			flag = 1;
			p = function[j].machine;
			while (p != NULL&&flag == 1)
			{
				//���������㷢�����ϣ��鿴��ǰ�Ƿ���Խ��и���
				if (machine[p->node_num].fault == 1)
				{
					change_machine(p->node_num, i);
				}

				//����������δ��������
				if (machine[p->node_num].fault == 0)
				{
					//�����������ȷ����������״̬
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

					//������ܽ��������ﵽ1
					if (function[j].capacity >= 1)
					{
						flag = 0;
					}
				}

				//��������������Ϊ0
				else
				{
					machine[p->node_num].used = 0;
				}

				p = p->next;
			}

			//�������û�����״̬
			while (p != NULL)
			{
				machine[p->node_num].used = 0;
				p = p->next;
			}

			//������ܽ�����
			if (function[j].capacity == 0)
			{
				node[j + N_process].fault = 1;
			}

			//������ܽ��δ����
			else
			{
				node[j + N_process].fault = 0;
			}
		}

		//�ж�ϵͳ�Ƿ������������
		judge_fault();
		for (j = 0; j < N_process_exist; j++)
		{
			result.fault[sequence_process[j]][i] += node[sequence_process[j]].fault;
		}

		//���ϵͳ������������
		if (node[sequence_process[N_process_exist]].fault == 0)
		{
			//�鿴�Ƿ�������δ�ﵽ1�Ĺ��ܽ��
			for (j = 0; j < N_function; j++)
			{
				if (function[j].capacity < system_capacity)
				{
					//�Ƚ��ù��ܽ����Ϊ���ϣ��ж�ϵͳ�Ƿ������������
					node[j].fault = 1;
					judge_fault();

					//����ù��ܽ�㲻�ɻ�ȱ
					if (node[sequence_process[N_process_exist]].fault == 1)
					{
						system_capacity = function[j].capacity;
					}

					//�����ܽ���Ϊ����
					node[j].fault = 0;
				}
			}

			//���»���������
			for (j = 0; j < N_machine; j++)
			{
				if (machine[j].used == 1)
				{
					if (machine[j].backup_recover_time_remaining > 0)
					{
						machine[j].backup_recover_time_remaining--;

						//����ָ���������������
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

		//���ϵͳ��������
		else
		{
			system_capacity = 0;

			for (j = 0; j < N_machine; j++)
			{
				machine[j].reliability_move_step++;
			}
		}
		result.capacity[i] += system_capacity;

		//����������
		for (j = 0; j < N_component; j++)
		{
			//����������м�
			for (k = 0; k < backup_place_sum - 1; k++)
			{
				//�鿴�Ƿ���Ҫ�������м������������
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

				//���±��ݾ����ʹ��ʱ��
				len_temp = component[j].backup_arrive_time_remaining[k].size();
				for (l = 0; l < len_temp; l++)
				{
					node_temp = component[j].backup_arrive_time_remaining[k].front() - 1;
					component[j].backup_arrive_time_remaining[k].pop();

					//��������ʹ�
					if (node_temp == 0)
					{
						component[j].backup_available_sum[k]++;
					}

					//�������δ�ʹ�
					else
					{
						component[j].backup_arrive_time_remaining[k].push(node_temp);
					}
				}
			}

			//���㴢�����
			for (k = 0; k < backup_place_sum; k++)
			{
				result.cost += component[j].backup_available_sum[k] * component[j].storage_cost_per_volume[k] * component[j].volume;
			}

			//�����𻵵���������޺õ�ʱ��
			len_temp = component[j].repair_recover_time_remaining.size();
			for (k = 0; k < len_temp; k++)
			{
				node_temp = component[j].repair_recover_time_remaining.front() - 1;
				component[j].repair_recover_time_remaining.pop();

				//����޺���
				if (node_temp == 0)
				{
					component[j].backup_available_sum[component[j].repair_place]++;

					//�ͷ���Դ
					p = component[j].resource;
					while (p != NULL)
					{
						resource[p->node_num].backup_available_sum[component[j].repair_place] += p->node_sum;
						p = p->next;
					}
				}

				//���δ�޺�
				else
				{
					component[j].repair_recover_time_remaining.push(node_temp);
				}
			}

			//�����𻵵���������ʹ��޸��ص��ʱ��
			len_temp = component[j].repair_arrive_time_remaining.size();
			for (k = 0; k < len_temp; k++)
			{
				node_temp = component[j].repair_arrive_time_remaining.front() - 1;
				component[j].repair_arrive_time_remaining.pop();

				//����ʹ���
				if (node_temp == 0)
				{
					component_waiting_for_repair[component[j].repair_place].push(j);
				}

				//���δ�ʹ�
				else
				{
					component[j].repair_arrive_time_remaining.push(node_temp);
				}
			}
		}

		//���µȴ��޸������
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


/**************************************************�������**************************************************/


//������
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

