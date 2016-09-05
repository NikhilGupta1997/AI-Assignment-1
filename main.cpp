#include <iostream>
#include <string>
#include <string.h>
#include <cstdlib>
#include <cstdio>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <time.h> 
#include <queue>
#include <fstream>

#define max 10000
using namespace std;

struct node{ // bid node structure
	int bid_id;
	int cid; //company
	double price;  
	int norc;  // no. of regions in one bid
	int *region;
	double score;
	bool used;
	vector<int> conflicting_bids;
};

struct region{
	int noOfBids;
	vector<int> bid_nos;
	bool used;
};

struct company{
	int noOfBids;
	vector<int> bid_nos;
	bool used;
};

struct CompareReg : public std::binary_function<region, region, bool>{
   bool operator()(const region lhs, const region rhs) const
   {
      return lhs.noOfBids < rhs.noOfBids;
   }
};

struct CompareScore : public std::binary_function<node, node, bool>{
   bool operator()(const node lhs, const node rhs) const
   {
      return lhs.score < rhs.score;
   }
};

int maxSteps = 500; //may vary for different inputs
int noOfSteps;

/*Probabilities multiplied by 100*/
int rp; //restart probability: 1/b
int wp = 10; //random walk probability: 1%
int np; //novelty probability(choosing highest when its age is not larger: 0.5-(age(b2)/age(b1) - 1)/(score(b1)/score(b2))

time_t t,start,check;
float tim; // time
int nor; // number of regions
int nob; // number of bids
int noc; // number of companies
struct node *tob, *stob; // total no. of bids // sorted list of bids
struct region *reg;
struct company *comp;
int *age; //age of all bids

//bool *com; //keeps record of which companies and regions can be selected in the remaining unprocessed bids
//(bid and reg not needed as bool value is incorporated into struct)

double temp,currentVal=0,maxVal = 0;//max value obtained until now

// Used to generate the Start State
priority_queue<region,vector<region>, CompareReg > region_size;

// Debugging perposes to get Bid info
void return_bid(int bidno){
	cout<<"BID "<<bidno<< " ";
	cout<<tob[bidno].bid_id<<" "<<tob[bidno].cid<<" "<<endl;
	cout<<"Regions ";
	for(int i=0; i<tob[bidno].norc; i++){
		cout<<tob[bidno].region[i]<<" ";
	}
	cout<<endl;
}

// Debugging perposes to remember the best set of bids
std::vector<int> vec;
void remember(){
	vec.clear();
	for(int i=0; i<nob; i++){
		if(tob[i].used){
			// vec.push_back(tob[i].bid_id);
			// vec.push_back(100*i);
			vec.push_back(i);
		}
	}
}

//function to take input - read from console by redirection
void readFile(char* inputfile){
	// open the file to be read from
	FILE *fid;   
	fid = fopen(inputfile,"r");

	fscanf(fid,"%f\n\n",&tim);
	fscanf(fid,"%d\n\n",&nor);
	fscanf(fid,"%d\n\n",&nob);
	fscanf(fid,"%d\n\n",&noc);

	rp = 1000/nob;
	if(rp<1)
		rp=1;
	//tim in seconds with a margin of 3 seconds, can change later depending on how often it is being checked (how often random restart is being called)
	tim *= 60;
	tim -= 1;

	tob = new node[nob];
	stob = new node[nob];
	age = new int[nob];
	comp = new company[noc];
	reg = new region[nor];

	for(int i=0;i<nob;i++){
		tob[i].bid_id = i;

		char ch[max];
		fscanf(fid,"%[^\n] \n", ch);

		int t=0;int j=0;
		char ch1[max];
		while(ch[t]!=' '){
			ch1[j]=ch[t];
			j++;t++;
		}
		
		ch1[j]='\0';
		tob[i].cid=atoi(ch1);
	
		ch1[0]='\0';j=0;t++;
		while(ch[t]!=' '){
			ch1[j]=ch[t];
			j++;t++;
		}
		ch1[j]='\0';			
		tob[i].price=strtod (ch1, NULL);
		t++;
		
		int x=0;
		int w=t;		
		while(ch[t]!='#'){
			if(ch[t]==' ')
			{	x++;}
			t++;
		}
		tob[i].norc=x;
		tob[i].score = tob[i].price/tob[i].norc;
		t=w;
		tob[i].region = new int [x];
		for(int qq=0;qq<x;qq++){
			ch1[0]='\0';j=0;
			while(ch[t]!=' '){
				ch1[j]=ch[t];
				j++;t++;
			}
			t++;
			ch1[j]='\0';
			tob[i].region[qq]=atoi(ch1);
		}
		tob[i].used = false;
		stob[i] = tob[i];
	}
	fclose (fid);
}

void getStartState(); // 0.65 - 0.96
void getStartState1(); // 0.95 - 1.36
void getStartState2(); // 1.01 - 1.29
void getStartState3(); // 1.11 - 1.38
void fill(int);
bool checkReg(int);

// int not_found(std::vector<int> v, int value){
// 	for(int i=0; i<v.size(); i++){
// 		if(value == v[i]){
// 			return 0;
// 		}
// 	}
// 	return 1;
// }

// double loss(int bidno){
// 	double loss_amount = 0.0;
// 	int visited[nob];
// 	for(int i=0; i<nob; i++){
// 		visited[i] = 0;
// 	}
// 	for(int i=0;i<tob[bidno].norc;i++){
// 		int reg_no = tob[bidno].region[i];
// 		if(reg[reg_no].used){
// 			for(int j=0;j<reg[reg_no].noOfBids;j++){
// 				int b_no = reg[reg_no].bid_nos[j];
// 				if(tob[b_no].used && visited[b_no] == 0){
// 					visited[b_no] = 1;
// 					loss_amount +=tob[b_no].price;
// 					break;
// 				}
// 			}
// 		} 
// 	}
// 	// cout<<"Loss is "<<loss_amount<<endl;
// 	return loss_amount;
// }

double loss(int bidno) {
	double loss_amount = 0.0;
	int size = tob[bidno].conflicting_bids.size();
	int i,b_temp;
	for(i=0;i<size;i++) {
		b_temp = tob[bidno].conflicting_bids[i];
		if(tob[b_temp].used)
			loss_amount += tob[b_temp].price;
	}
	return loss_amount;
}

void remove(int comid){
	int index, i;
	for(i=0; i<nob; i++){
		if(tob[i].cid == comid){
			if(tob[i].used){
				index = i;
				break;
			}
		}
	}
	if (i<nob){
		currentVal-=tob[index].price;
		tob[index].used=false;
		int reg_no;
		comp[tob[index].cid].used = false;
		for(int i=0; i<tob[index].norc; i++){
			reg_no = tob[index].region[i];
			reg[reg_no].used=false;
		}
	}
}



void randomStart() //Whenever there is a random restart
{
	int i;
	int p,b1,b2, b1_map, b2_map;
	bool srand_call = false;
	do {
		// cout<<"Random Restart"<<endl;
		// remember();

		// for(i=0; i<vec.size(); i++){
		// 	cout<<vec[i]<<" ";
		// }
		// cout<<endl;



		start:
		time(&t);
		if(t-start>=tim)
			return;
		if(((t - start) % 20 == 0)&&!srand_call) {
			srand_call = true;
			srand(time(0));
		} else {
			srand_call = false;
		}

		for(i=0;i<nob;i++) {
			age[i] = maxSteps;
			tob[i].used = false;
		}
		for(i=0;i<nor;i++) {
			reg[i].used = false;
			region_size.push(reg[i]);
		}
		for(i=0;i<noc;i++) {
			comp[i].used = false;
		}
		noOfSteps = maxSteps;
		currentVal = 0;
		getStartState3();

		// // To see value of random start state
		// double total_price = 0.0;
		// for(int i=0; i<nob; i++){
		// 	if(tob[i].used==true){
		// 		total_price += tob[i].price;
		// 	}
		// }
		// cout<<total_price<<endl;
		
		// Hill Climbing
		do{
			time(&check);
			if(check-start>=tim)
				return;

			// cout<<maxVal<<" "<<noOfSteps<<endl;

			p = rand()%100;
			if(p<wp){
				do{
					// cout<<"Random Walk"<<endl;
					b1=rand()%nob;
				}while(tob[b1].used);
				remove(tob[b1].cid);
				fill(b1);
			} 
			else {
				b1 = 0;
				b1_map = stob[b1].bid_id;
				while(tob[b1_map].used || loss(b1_map)>tob[b1_map].price || comp[tob[b1_map].cid].used){
					b1++;
					// cout<<" b1 = "<<b1<<endl;
					if(b1>=nob){
						goto start;
					}
					b1_map = stob[b1].bid_id;
				}
				// cout<<tob[b1_map].price<<"***************"<<endl;
				b2 = b1+1;
				if(b2 >= nob){
					goto only_one;
				}
				b2_map = stob[b2].bid_id;
				while(tob[b2_map].used || loss(b2_map)>tob[b2_map].price || comp[tob[b2_map].cid].used){
					b2++;
					// cout<<" b2 = "<<b2<<endl;
					if(b2>=nob){
						goto only_one;
					}
					b2_map = stob[b2].bid_id;
				}
				// cout<<tob[b2_map].price<<"***************"<<endl;
				if(age[b1]>age[b2]){
					// cout<<"################### FILLING"<<endl;
					//remove(tob[b1_map].cid);
					fill(b1_map);
				} 
				else{
					p = rand()%100;	
					temp = 0.5 - (age[b2]*1.0/age[b1] - 1)*tob[b2_map].score/tob[b1_map].score;
					np = temp*100;
					if(p<np){
						// cout<<"################### FILLING"<<endl;
						only_one:
						// cout<<"Moved to only_one"<<endl;
						//remove(tob[b1_map].cid);
						fill(b1_map);
					} 
					else{
						// cout<<"################### FILLING"<<endl;
						//remove(tob[b2_map].cid);
						fill(b2_map);
					}
				}
			}
			p = rand()%1000;
		} while(p>=rp);
	} while(true);
}

// Helper function to get new start state
void getStartState(){
	int rand1, rand2, rand3, rand_no, num1, num2, num3;
	while ( !region_size.empty() ){
		region temp = region_size.top();
		region_size.pop();
		if(!temp.used){
			rand1 = rand()%temp.bid_nos.size();
			num1 = temp.bid_nos[rand1];
			rand2 = rand()%temp.bid_nos.size();
			num2 = temp.bid_nos[rand2];
			rand3 = rand()%temp.bid_nos.size();
			num3 = temp.bid_nos[rand3];
			rand_no = rand()%3;
			if(rand_no == 0){
				if(!checkReg(num1) && !comp[temp.bid_nos[rand1]].used)
					fill(num1);
			}
			else if(rand_no == 1){
				if(!checkReg(num2) && !comp[temp.bid_nos[rand2]].used)
					fill(num2);
			}
			else if(rand_no == 2){
				if(!checkReg(num3) && !comp[temp.bid_nos[rand3]].used)
					fill(num3);
			}
		}
	}
}

// Helper function to get new start state
void getStartState1(){
	int num1,i;
	num1=rand()%nob;
	fill(num1);
	for(i=(num1+1)%nob;i!=num1;i=(i+1)%nob){
		if(comp[tob[i].cid].used || checkReg(i))
			continue;
		fill(i);
	}
}

// Helper function to get new start state
void getStartState2(){
	int num = rand()%noc;
	int max_id=0;
	double max_score=0.0;
	for(int i=0; i<noc; i++){
		int no_of_bids = comp[num].noOfBids;
		for(int j=0; j<no_of_bids; j++){
			if(tob[comp[num].bid_nos[j]].score>max_score){
				max_id = comp[num].bid_nos[j];
				max_score = tob[comp[num].bid_nos[j]].score;
			}
		}
		int prob = rand()%100;
		if(prob>=0){
			if(!checkReg(max_id)){
				fill(max_id);
				continue;
			}
		}
		int random_bid1 = rand()%no_of_bids;
		int random_bid2 = rand()%no_of_bids;
		int random_bid3 = rand()%no_of_bids;
		int bid1 = comp[num].bid_nos[random_bid1];
		int bid2 = comp[num].bid_nos[random_bid2];
		int bid3 = comp[num].bid_nos[random_bid3];
		node first, second, third;
		if(tob[bid1].score>tob[bid2].score){
			if(tob[bid2].score>tob[bid3].score){
				first = tob[bid1];
				second = tob[bid2];
				third = tob[bid3];
			}
			else if(tob[bid1].score > tob[bid3].score){
				first = tob[bid1];
				second = tob[bid3];
				third = tob[bid2];
			}
			else{
				first = tob[bid3];
				second = tob[bid1];
				third = tob[bid2];
			}
		}
		else{
			if(tob[bid1].score>tob[bid3].score){
				first = tob[bid2];
				second = tob[bid1];
				third = tob[bid3];
			}
			else if(tob[bid2].score > tob[bid3].score){
				first = tob[bid2];
				second = tob[bid3];
				third = tob[bid1];
			}
			else{
				first = tob[bid3];
				second = tob[bid2];
				third = tob[bid1];
			}
		}
		if(!checkReg(first.bid_id)){
			fill(first.bid_id);
		}
		else if(!checkReg(second.bid_id)){
			fill(second.bid_id);
		}
		else if(!checkReg(third.bid_id)){
			fill(third.bid_id);
		}
		num = (num+1)%noc;
	}
}

bool not_zero(bool arr[]){
	for(int i=0; i<noc; i++){
		if (arr[i])
			return true;
	}
	return false;
}

void getStartState3(){
	priority_queue<node,vector<node>, CompareScore > company_scores[max];
	for(int j=0; j<nob; j++){
		int co = tob[j].cid;
		company_scores[co].push(tob[j]);
	}
	int num = rand()%noc;
	bool count[noc];
	for(int i=0; i<noc; i++){
		count[i] = true;
	}
	while(not_zero(count)){
		count[num]=false;
		while(!company_scores[num].empty()){
			node temp = company_scores[num].top();
			company_scores[num].pop();
			int id = temp.bid_id;
			int p = rand()%1000;
			if(p>800)
				continue;
			if(!checkReg(id)){
				fill(id);
				break;
			}
		}
		num = (num+1)%noc;
		//num = rand()%noc;
	}	
}

//helper function of randomStart function
// void fill(int bidno){	
// 	noOfSteps--;
// 	age[bidno] = noOfSteps;
// 	comp[tob[bidno].cid].used=true;
// 	int i,j;
// 	int reg_no,b_no;
// 	for(i=0;i<tob[bidno].norc;i++){
// 		reg_no = tob[bidno].region[i];
// 		if(reg[reg_no].used){
// 			for(j=0;j<reg[reg_no].noOfBids;j++){
// 				b_no = reg[reg_no].bid_nos[j];
// 				if(tob[b_no].used){
// 					tob[b_no].used = false;
// 					comp[tob[b_no].cid].used = false;
// 					for(int k=0; k<tob[b_no].norc; k++){
// 						reg[tob[b_no].region[k]].used = false;
// 					}
// 					reg[reg_no].used = true;
// 					currentVal -= tob[b_no].price;
// 					break;
// 				}
// 			}
// 		} 
// 		else {
// 			reg[reg_no].used = true;
// 		}
// 	}
// 	currentVal += tob[bidno].price;
// 	tob[bidno].used=true;
// 	if(currentVal>maxVal){
// 		maxVal = currentVal;
// 		remember();
// 	}
// }

void fill(int bidno){	
	noOfSteps--;
	age[bidno] = noOfSteps;
	// if(comp[tob[bidno].cid].used)
	// 	remove(tob[bidno].cid);
	// else
		comp[tob[bidno].cid].used=true;
	int i,j;
	int reg_no,b_no;
	int size = tob[bidno].conflicting_bids.size();
	// cout<<"size"<<size<<endl;
	for(i=0;i<size;i++) {
		b_no = tob[bidno].conflicting_bids[i];
		if(tob[b_no].used) {
			tob[b_no].used = false;
			comp[tob[b_no].cid].used = false;
			for(int k=0; k<tob[b_no].norc; k++){
				reg[tob[b_no].region[k]].used = false;
			}
			currentVal -= tob[b_no].price;
		} else {
			// cout<<"false"<<i<<endl;
		}
	}
	// cout<<""
	for(i=0;i<tob[bidno].norc;i++){
			reg_no = tob[bidno].region[i];
			reg[reg_no].used = true;
			// cout<<i<<endl;
		
	}

	// cout<<"exit loop"<<endl;
	currentVal += tob[bidno].price;
	tob[bidno].used=true;
	if(currentVal>maxVal){
		maxVal = currentVal;
		remember();
	}
}

//helper function of randomStart function
bool checkReg(int bidno){
	for(int i=0;i<tob[bidno].norc;i++){
		if(reg[tob[bidno].region[i]].used)
			return true;
	}
	return false;
}

bool compareBids(const node a, const node b){
	return a.score > b.score;
}

bool anyCommonRegion(int a, int b) {
	int i,j;
	for(i=0;i<tob[a].norc;i++) {
		for(j=0;j<tob[b].norc;j++) {
			if(tob[a].region[i]==tob[b].region[j])
				return true;
		}
	}
	return false;
}

int main(int argc, char* argv[]){
	time(&start);
	
	readFile(argv[1]);
	sort(stob,stob+nob,compareBids);
	int i,j;
	for(i=0;i<nor;i++) {
		reg[i].noOfBids = 0;
	}
	for(i=0; i<noc; i++){
		comp[i].noOfBids = 0;
	}
	for(i=0;i<nob;i++) {
		for(j=0;j<tob[i].norc;j++) {
			reg[tob[i].region[j]].bid_nos.push_back(i);
			reg[tob[i].region[j]].noOfBids++;
			reg[tob[i].region[j]].used = false;
			comp[tob[i].cid].bid_nos.push_back(i);
			comp[tob[i].cid].noOfBids++;
			comp[tob[i].cid].used = false;
		}
		for(j=i+1;j<nob;j++) {
			if(tob[j].cid==tob[i].cid) {
				tob[i].conflicting_bids.push_back(j);
				tob[j].conflicting_bids.push_back(i);
				continue;
			} else if (anyCommonRegion(i,j)) {
				tob[i].conflicting_bids.push_back(j);
				tob[j].conflicting_bids.push_back(i);
			}
		}
	}
	randomStart();
	
	cout<<maxVal<<endl;
	ofstream outputFile;
	outputFile.open(argv[2]);
	for(i=0; i<vec.size(); i++){
		outputFile<<tob[vec[i]].bid_id<<" ";
	}
	outputFile<<"#";
	outputFile.close();
	
	return 0;
}
