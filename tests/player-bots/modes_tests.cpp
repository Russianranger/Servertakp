#include "player_bot_formation.h"
#include <cassert>
#include <iostream>
int main(){using namespace BotFormation;int checks=0;
for(unsigned n=1;n<=5;++n)for(bool gather:{false,true})for(bool line:{false,true})for(float size:{3.f,6.f,9.f,15.f}) {
std::vector<float> sizes(n,size);auto points=Layout(sizes,size,gather,line);assert(points.size()==n);
for(unsigned i=0;i<n;++i){auto p=points[i];assert(std::hypot(p.x,p.y)+.001>=2*BodyRadius(size)+3);++checks;
if(line){assert(p.x==0);assert(i==0||p.y<points[i-1].y);}
for(unsigned j=0;j<i;++j){assert(std::hypot(p.x-points[j].x,p.y-points[j].y)+.001>=2*BodyRadius(size)+3);++checks;}}
}
for(unsigned n=1;n<=5;++n){std::vector<float> s(n,6);auto a=Layout(s,6,false,false),b=Layout(s,6,true,false);assert(std::abs(b.back().y)<std::abs(a.back().y));}
std::cout<<checks<<" spacing checks passed\n";}
