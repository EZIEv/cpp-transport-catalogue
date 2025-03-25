#include <iostream>

#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

using namespace std;

int main() {
    request_handler::RequestHandler rh;
    map_renderer::MapRenderer mr;

    json_reader::ParseRequest(cin, rh, mr);
    rh.ApplyBaseRequests();

    json_reader::PrintStat(cout, rh.ApplyStatRequests(mr));
}