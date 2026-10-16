#include "mission_presentation.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm;
int main(int argc,char **argv) try {
    if(argc!=2) throw std::invalid_argument("resource root required");
    const auto resources=skirmish::mapCampaignResources(argv[1]);
    skirmish::MissionPresentation presentation(*resources);
    if(presentation.terrain().empty()) throw std::runtime_error("empty terrain presentation");
    for(const game::Point cell:std::vector<game::Point>{{2,5,3},{3,5,3},{1,6,3},{6,1,3}}) {
        if(presentation.pick(presentation.center(cell),{})!=std::optional<game::Point>(cell))
            throw std::runtime_error("native standing cell marker does not pick its cell");
        if(presentation.pick(presentation.unitCenter(cell),{cell})!=std::optional<game::Point>(cell))
            throw std::runtime_error("native glyph marker does not pick its actor cell");
    }
    if(presentation.pick({-1,0},{}) || presentation.pick(presentation.center({0,0,3}),{}))
        throw std::runtime_error("outside/sealed cell was picked");
    std::cout<<"terrain images and native marker picking passed\n";
} catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
