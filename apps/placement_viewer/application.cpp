#include "application.h"

#include <fstream>


#include "../../src/timing/liberty.h"

#include "../../src/timing/graph_builder.h"

#include "../../src/standard_cell/standard_cells.h"

#include "../../src/parsing/def.h"
#include "../../src/parsing/lef.h"
#include "../../src/parsing/verilog.h"

#include <omp.h>

application::application(const std::string v_file, const std::string lef_file, const std::string def_file, const std::string lib_file):
    m_netlist(&m_std_cells),
    m_library(&m_std_cells),
    m_placement(&m_netlist, &m_library),
    m_timing_arcs(&m_std_cells),
    m_timing_library(&m_timing_arcs, &m_std_cells)


{


    std::cout << "reading inputs..." << std::endl;


//    ophidian::placement::lef::read(dot_lef, &m_std_cells, &m_library); TODO READ LEF
//    ophidian::placement::def::read(dot_def, &m_netlist, &m_placement, &m_floorplan); TODO READ DEF
    ophidian::timing::liberty::read(lib_file, m_timing_library);


    std::cout << "setting design constraints..." << std::endl;
    ophidian::timing::default_design_constraints default_dc(m_netlist);
    ophidian::timing::design_constraints dc(default_dc.dc());
    for(auto driver : dc.input_drivers)
        m_std_cells.pin_direction(m_netlist.pin_std_cell(m_netlist.pin_by_name(driver.port_name)), ophidian::standard_cell::pin_directions::OUTPUT);

    m_std_cells.pin_direction(m_netlist.pin_std_cell(m_netlist.pin_by_name(dc.clock.port_name)), ophidian::standard_cell::pin_directions::OUTPUT);

    for(auto out_load : dc.output_loads)
    {
        auto PO_pin = m_netlist.pin_by_name(out_load.port_name);
        auto PO_std_cell_pin = m_netlist.pin_std_cell(PO_pin);
        m_std_cells.pin_direction(m_netlist.pin_std_cell(PO_pin), ophidian::standard_cell::pin_directions::INPUT);
        m_timing_library.pin_capacitance(PO_std_cell_pin, boost::units::quantity<boost::units::si::capacitance>(out_load.pin_load*boost::units::si::femto*boost::units::si::farads));
    }


    std::cout << "building graph..." << std::endl;
    ophidian::timing::graph_builder::build(m_netlist, m_timing_library, dc, m_graph);


    std::cout << "initializing routing estimator & timing analysis " << std::endl;
    m_flute.reset(new ophidian::timingdriven_placement::sta_flute_net_calculator(m_graph, m_placement, m_timing_library, m_netlist));
//    m_sta.reset(new ophidian::timing::static_timing_analysis(m_graph, m_timing_library, m_flute.get()));


//    ophidian::timing::static_timing_analysis * sta = m_sta.get();
//    for(auto net : m_netlist.net_system())
//        sta->make_dirty(net.first);
//    sta->set_constraints( m_netlist, dc );

//    std::cout << "building RTree indexing..." << std::endl;
//    for (auto c : m_netlist.cell_system()) {
//        place_cell_and_update_index(c.second,
//                                    m_placement.cell_position(c.second));
//    }

//    omp_set_num_threads(omp_get_max_threads());

}

application::~application() {
}


std::vector<rtree_node> application::create_rtree_nodes(
        ophidian::entity_system::entity cell) {

    auto geometry = m_placement.cell_geometry(cell);
    std::vector<rtree_node> nodes;
    for (auto& poly : geometry) {
        box cellbox;
        boost::geometry::envelope(poly, cellbox);
        rtree_node cell_node(cellbox, cell);
        nodes.push_back(cell_node);
    }
    return nodes;
}


void application::place_cell_and_update_index(ophidian::entity_system::entity cell,
                                              point position) {
    // remove from index
    std::vector<rtree_node> nodes = create_rtree_nodes(cell);
    if (!m_position2cellentity.empty()) {
        m_position2cellentity.remove(nodes);
    }

    m_placement.cell_position(cell, position);

    // update index
    nodes.clear();
    nodes = create_rtree_nodes(cell);
    m_position2cellentity.insert(nodes.begin(), nodes.end());


    auto cell_pins = m_netlist.cell_pins(cell);
    for(auto pin : cell_pins)
    {
        auto net = m_netlist.pin_net(pin);
//        m_sta->make_dirty(net);
    }

}

ophidian::entity_system::entity application::get_cell(point position) const
{
    std::vector<rtree_node> result;
    m_position2cellentity.query(boost::geometry::index::contains(position),
                                std::back_inserter(result));
    if(result.empty())
        return ophidian::entity_system::entity{};
    return result.front().second;
}

bool application::cell_std_cell(ophidian::entity_system::entity cell, std::string std_cell_name)
{
    auto old_rtree_nodes = create_rtree_nodes(cell);
    bool result = m_netlist.cell_std_cell(cell, std_cell_name);
    if (result) {
        m_position2cellentity.remove(old_rtree_nodes);
        auto new_rtree_nodes = create_rtree_nodes(cell);
        m_position2cellentity.insert(new_rtree_nodes.begin(), new_rtree_nodes.end());
    }
    return result;
}

boost::units::quantity<boost::units::si::time> application::cell_worst_slack(ophidian::entity_system::entity cell) const
{
//    boost::units::quantity<boost::units::si::time> worst = std::numeric_limits<boost::units::quantity<boost::units::si::time> >::infinity();
//    auto cell_pins = m_netlist.cell_pins(cell);
//    for(auto pin : cell_pins)
//    {
//        auto pin_worst = std::min(m_sta->rise_slack(pin), m_sta->fall_slack(pin));
//        worst = std::min(worst, pin_worst);
//    }

//    return worst;
}

void application::run_sta()
{
    std::cout << "running STA..." << std::endl;
//    m_sta->run();


}

boost::units::quantity<boost::units::si::time> application::rise_arrival(ophidian::entity_system::entity pin) const
{
//    return m_sta->rise_arrival(pin);
}

boost::units::quantity<boost::units::si::time> application::fall_arrival(ophidian::entity_system::entity pin) const
{
//    return  m_sta->fall_arrival(pin);
}

boost::units::quantity<boost::units::si::time> application::fall_slack(ophidian::entity_system::entity pin) const
{
//    return m_sta->fall_slack(pin);
}

std::vector<ophidian::entity_system::entity> application::critical_path() const
{
//    return m_sta->critical_path();

}

boost::units::quantity<boost::units::si::time> application::rise_slack(ophidian::entity_system::entity pin) const
{
//    return m_sta->rise_slack(pin);
}
