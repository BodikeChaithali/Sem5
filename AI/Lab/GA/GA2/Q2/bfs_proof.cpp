#include <bits/stdc++.h>
using namespace std;
// Exhaustive BFS over the extended-scenario state space.
// Locations: 0 Base,1 A,2 B,3 C,4 D,5 E,6 F.  Rock position: 0-6 = location, 7 = Bay1, 8 = Bay2.
int NR=7;
vector<pair<int,int>> edges={{0,1},{1,0},{1,3},{3,1},{2,4},{4,2},{3,5},{5,3},{4,5},{5,4},{5,6},{6,5},{3,4}}; // Base<->B closed, C->D one-way
bool hard[7]={0,0,1,0,1,0,1};  // rock3, rock5, rock7 are hard
typedef unsigned long long u64;
u64 enc(int rv,int dr,int*r){u64 x=rv*2+dr;for(int i=0;i<NR;i++)x=x*9+r[i];return x;}
void dec(u64 x,int&rv,int&dr,int*r){for(int i=NR-1;i>=0;i--){r[i]=x%9;x/=9;}dr=x%2;x/=2;rv=x;}
int main(){
  int init[7]={1,2,3,4,6,5,6}; // rock1 A, rock2 B, rock3 C, rock4 D, rock5 F, rock6 E, rock7 F
  u64 total=7*2; for(int i=0;i<NR;i++) total*=9;
  vector<signed char> dist(total,-1);
  u64 s0=enc(0,0,init); dist[s0]=0;
  vector<u64> cur={s0},nxt; int d=0; u64 count=1;
  while(!cur.empty()){
    for(u64 s:cur){
      int rv,dr,r[7]; dec(s,rv,dr,r);
      bool goal=(rv==0); for(int i=0;i<NR;i++) if(r[i]!=0) goal=false;
      if(goal){printf("GOAL first reached at depth %d (shortest possible plan length). States visited: %llu\n",d,count);return 0;}
      auto push=[&](int rv2,int dr2,int*r2){u64 t=enc(rv2,dr2,r2); if(dist[t]<0){dist[t]=d+1;nxt.push_back(t);count++;}};
      int b1=-1,b2=-1; for(int i=0;i<NR;i++){if(r[i]==7)b1=i; if(r[i]==8)b2=i;}
      for(auto&e:edges) if(e.first==rv) push(e.second,dr,r);
      if(dr==0&&rv==5) push(rv,1,r);
      for(int i=0;i<NR;i++){
        if(r[i]==rv && (!hard[i]||dr==1)){
          if(b1<0){int q[7];memcpy(q,r,sizeof q);q[i]=7;push(rv,dr,q);}
          if(b2<0){int q[7];memcpy(q,r,sizeof q);q[i]=8;push(rv,dr,q);}
        }
        if(r[i]==7||r[i]==8){int q[7];memcpy(q,r,sizeof q);q[i]=rv;push(rv,dr,q);}
      }
    }
    cur.swap(nxt); nxt.clear(); d++;
  }
  printf("no goal reachable\n");
}
