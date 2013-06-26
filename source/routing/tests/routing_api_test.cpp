#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_navimake
#include <boost/test/unit_test.hpp>
#include "routing/raptor_api.h"
#include "naviMake/build_helper.h"


using namespace navitia;
using namespace routing;
using namespace boost::posix_time;

BOOST_AUTO_TEST_CASE(simple_journey){
    std::vector<std::string> forbidden;
    navimake::builder b("20120614");
    b.vj("A")("stop_area:stop1", 8*3600 +10*60, 8*3600 + 11 * 60)("stop_area:stop2", 8*3600 + 20 * 60 ,8*3600 + 21*60);
    type::Data data;
    b.generate_dummy_basis();
    b.data.pt_data.index();
    b.data.build_raptor();
    b.data.build_uri();
    b.data.meta.production_date = boost::gregorian::date_period(boost::gregorian::date(2012,06,14), boost::gregorian::days(7));
    RAPTOR raptor(b.data);

    type::EntryPoint origin("stop_area:stop1");
    type::EntryPoint destination("stop_area:stop2");

    streetnetwork::StreetNetwork sn_worker(data.geo_ref);
    pbnavitia::Response resp = make_response(raptor, origin, destination, {"20120614T021000"}, true, 1.38, 1000, false, forbidden, sn_worker);

    BOOST_REQUIRE_EQUAL(resp.response_type(), pbnavitia::ITINERARY_FOUND);
    BOOST_REQUIRE_EQUAL(resp.journeys_size(), 1);
    pbnavitia::Journey journey = resp.journeys(0);

    BOOST_REQUIRE_EQUAL(journey.sections_size(), 1);
    pbnavitia::Section section = journey.sections(0);

    BOOST_REQUIRE_EQUAL(section.stop_date_times_size(), 2);
    auto st1 = section.stop_date_times(0);
    auto st2 = section.stop_date_times(1);
    BOOST_CHECK_EQUAL(st1.stop_point().uri(), "stop_area:stop1");
    BOOST_CHECK_EQUAL(st2.stop_point().uri(), "stop_area:stop2");
    BOOST_CHECK_EQUAL(st1.departure_date_time(), "20120614T081100");
    BOOST_CHECK_EQUAL(st2.arrival_date_time(), "20120614T082000");
}

BOOST_AUTO_TEST_CASE(journey_array){
    std::vector<std::string> forbidden;
    navimake::builder b("20120614");
    b.vj("A")("stop_area:stop1", 8*3600 +10*60, 8*3600 + 11 * 60)("stop_area:stop2", 8*3600 + 20 * 60 ,8*3600 + 21*60);
    b.vj("A")("stop_area:stop1", 9*3600 +10*60, 9*3600 + 11 * 60)("stop_area:stop2",  9*3600 + 20 * 60 ,9*3600 + 21*60);
    type::Data data;
    b.generate_dummy_basis();
    b.data.pt_data.index();
    b.data.build_raptor();
    b.data.build_uri();
    b.data.meta.production_date = boost::gregorian::date_period(boost::gregorian::date(2012,06,14), boost::gregorian::days(7));
    RAPTOR raptor(b.data);

    type::EntryPoint origin("stop_area:stop1");
    type::EntryPoint destination("stop_area:stop2");

    streetnetwork::StreetNetwork sn_worker(data.geo_ref);

    // On met les horaires dans le desordre pour voir s'ils sont bien triÃ© comme attendu
    std::vector<std::string> datetimes({"20120614T080000", "20120614T090000"});

    pbnavitia::Response resp = make_response(raptor, origin, destination, datetimes, true, 1.38, 1000, false, forbidden, sn_worker);

    BOOST_REQUIRE_EQUAL(resp.response_type(), pbnavitia::ITINERARY_FOUND);
    BOOST_REQUIRE_EQUAL(resp.journeys_size(), 2);

    pbnavitia::Journey journey = resp.journeys(0);
    BOOST_REQUIRE_EQUAL(journey.sections_size(), 1);
    pbnavitia::Section section = journey.sections(0);
    BOOST_REQUIRE_EQUAL(section.stop_date_times_size(), 2);
    auto st1 = section.stop_date_times(0);
    auto st2 = section.stop_date_times(1);
    BOOST_CHECK_EQUAL(st1.stop_point().uri(), "stop_area:stop1");
    BOOST_CHECK_EQUAL(st2.stop_point().uri(), "stop_area:stop2");
    BOOST_CHECK_EQUAL(st1.departure_date_time(), "20120614T081100");
    BOOST_CHECK_EQUAL(st2.arrival_date_time(), "20120614T082000");

    journey = resp.journeys(1);
    BOOST_REQUIRE_EQUAL(journey.sections_size(), 1);
    section = journey.sections(0);
    BOOST_REQUIRE_EQUAL(section.stop_date_times_size(), 2);
    st1 = section.stop_date_times(0);
    st2 = section.stop_date_times(1);
    BOOST_CHECK_EQUAL(st1.stop_point().uri(), "stop_area:stop1");
    BOOST_CHECK_EQUAL(st2.stop_point().uri(), "stop_area:stop2");
    BOOST_CHECK_EQUAL(st1.departure_date_time(), "20120614T091100");
    BOOST_CHECK_EQUAL(st2.arrival_date_time(), "20120614T092000");
}

BOOST_AUTO_TEST_CASE(journey_streetnetworkmode){

/*

   K  ------------------------------ J
      |                             |
      |                             |
      |                             |  I
      |                             ---------------- H
      |                                             |
      |                                             |
      |                                             |
      |                                             |
      |                                             | g
      |                                             ---------------------- A --------------------------------R
      |                                                               /  |
      |                                                             /    |
      |                                                           /      |
      |                                                         /        |
      |                                                       /          |
      |                                                     /            |
      |                                                   /              | E
      |                                                 /                ------------------------- F
      |                                              /                                           |
      |                                          /                                               |
      |                                       /                                                  |
      |                                    /                                                     |
      |                                 /                                                        |
      |                             /                                                            |
      |                          /                                                               |
      |                      /                                                                   |
      |                  /                                                                       |
      |             /                                                                            |
      B------------------------------------------------------------------------------------------- C
      |
     |
      |
      S

        On veut aller de S vers R :
            *) la voie cyclable est : A->G->H->I->J->K->B
            *) la voie rÃ©servÃ©e Ã  la voiture est : A->E->F->C->B
            *) la voie MAP est : A->B
            *) la voie cyclable, voiture et MAP : S->B
            *) entre A et B que le transport en commun



            Coordonées :
                        A(12, 8)    0
                        G(10, 8)    1
                        H(10, 10)   2
                        I(7, 10)    3
                        J(7, 12)    4
                        K(1, 12)    5
                        B(1, 2)     6
                        C(15, 2)    7
                        F(15, 5)    8
                        E(12, 5)    9
                        R(21, 8)    10
                        S(1, 1)     11



*/
    namespace ng = navitia::georef;
    int AA = 0;
    int GG = 1;
    int HH = 2;
    int II = 3;
    int JJ = 4;
    int KK = 5;
    int BB = 6;
    int CC = 7;
    int FF = 8;
    int EE = 9;
    int RR = 10;
    int SS = 11;
    navimake::builder b("20120614");

    type::GeographicalCoord A(12, 8, false);
    boost::add_vertex(ng::Vertex(A),b.data.geo_ref.graph);

    type::GeographicalCoord G(10, 8, false);
    boost::add_vertex(ng::Vertex(G),b.data.geo_ref.graph);

    type::GeographicalCoord H(10, 10, false);
    boost::add_vertex(ng::Vertex(H),b.data.geo_ref.graph);

    type::GeographicalCoord I(7, 10, false);
    boost::add_vertex(ng::Vertex(I),b.data.geo_ref.graph);

    type::GeographicalCoord J(7, 12, false);
    boost::add_vertex(ng::Vertex(J),b.data.geo_ref.graph);

    type::GeographicalCoord K(1, 12, false);
    boost::add_vertex(ng::Vertex(K),b.data.geo_ref.graph);

    type::GeographicalCoord B(1, 2, false);
    boost::add_vertex(ng::Vertex(B),b.data.geo_ref.graph);

    type::GeographicalCoord C(15, 2, false);
    boost::add_vertex(ng::Vertex(C),b.data.geo_ref.graph);

    type::GeographicalCoord F(15, 5, false);
    boost::add_vertex(ng::Vertex(F),b.data.geo_ref.graph);

    type::GeographicalCoord E(12, 5, false);
    boost::add_vertex(ng::Vertex(E),b.data.geo_ref.graph);

    type::GeographicalCoord R(21, 8, false);
    boost::add_vertex(ng::Vertex(R),b.data.geo_ref.graph);

    type::GeographicalCoord S(1, 1, false);
    boost::add_vertex(ng::Vertex(S),b.data.geo_ref.graph);
    // Pour le vls
    type::GeographicalCoord V(0.5, 1, false);
    type::GeographicalCoord Q(18, 10, false);


    ng::vertex_t Conunt_v = boost::num_vertices(b.data.geo_ref.graph);
    b.data.geo_ref.init_offset(Conunt_v);
    ng::Way* way;

    way = new ng::Way();
    way->name = "rue ab"; // A->B
    way->idx = 0;
    way->way_type = "rue";
    b.data.geo_ref.ways.push_back(way);

    way = new ng::Way();
    way->name = "rue ae"; // A->E
    way->idx = 1;
    way->way_type = "rue";
    b.data.geo_ref.ways.push_back(way);

    way = new ng::Way();
    way->name = "rue ef"; // E->F
    way->idx = 2;
    way->way_type = "rue";
    b.data.geo_ref.ways.push_back(way);

    way = new ng::Way();
    way->name = "rue fc"; // F->C
    way->idx = 3;
    way->way_type = "rue";
    b.data.geo_ref.ways.push_back(way);

    way = new ng::Way();
    way->name = "rue cb"; // C->B
    way->idx = 4;
    way->way_type = "rue";
    b.data.geo_ref.ways.push_back(way);

    way = new ng::Way();
    way->name = "rue ag"; // A->G
    way->idx = 5;
    way->way_type = "rue";
    b.data.geo_ref.ways.push_back(way);

    way = new ng::Way();
    way->name = "rue gh"; // G->H
    way->idx = 6;
    way->way_type = "rue";
    b.data.geo_ref.ways.push_back(way);

    way = new ng::Way();
    way->name = "rue hi"; // H->I
    way->idx = 7;
    way->way_type = "rue";
    b.data.geo_ref.ways.push_back(way);

    way = new ng::Way();
    way->name = "rue ij"; // I->J
    way->idx = 8;
    way->way_type = "rue";
    b.data.geo_ref.ways.push_back(way);

    way = new ng::Way();
    way->name = "rue jk"; // J->K
    way->idx = 9;
    way->way_type = "rue";
    b.data.geo_ref.ways.push_back(way);

    way = new ng::Way();
    way->name = "rue kb"; // K->B
    way->idx = 10;
    way->way_type = "rue";
    b.data.geo_ref.ways.push_back(way);

    way = new ng::Way();
    way->name = "rue ar"; // A->R
    way->idx = 11;
    way->way_type = "rue";
    b.data.geo_ref.ways.push_back(way);

    way = new ng::Way();
    way->name = "rue bs"; // B->S
    way->idx = 12;
    way->way_type = "rue";
    b.data.geo_ref.ways.push_back(way);

// A->B
    boost::add_edge(AA, BB, ng::Edge(0,10), b.data.geo_ref.graph);
    boost::add_edge(BB, AA, ng::Edge(0,10), b.data.geo_ref.graph);
    b.data.geo_ref.ways[0]->edges.push_back(std::make_pair(AA, BB));
    b.data.geo_ref.ways[0]->edges.push_back(std::make_pair(BB, AA));

// A->E
    boost::add_edge(AA , EE, ng::Edge(1,5), b.data.geo_ref.graph);
    boost::add_edge(EE , AA, ng::Edge(1,5), b.data.geo_ref.graph);
    boost::add_edge(AA + b.data.geo_ref.car_offset, EE + b.data.geo_ref.car_offset, ng::Edge(1,5), b.data.geo_ref.graph);
    boost::add_edge(EE + b.data.geo_ref.car_offset, AA + b.data.geo_ref.car_offset, ng::Edge(1,5), b.data.geo_ref.graph);
    b.data.geo_ref.ways[1]->edges.push_back(std::make_pair(AA, EE));
    b.data.geo_ref.ways[1]->edges.push_back(std::make_pair(EE, AA));

// E->F
    boost::add_edge(EE , FF , ng::Edge(2,5), b.data.geo_ref.graph);
    boost::add_edge(FF , EE , ng::Edge(2,5), b.data.geo_ref.graph);
    boost::add_edge(EE + b.data.geo_ref.car_offset, FF + b.data.geo_ref.car_offset, ng::Edge(2,5), b.data.geo_ref.graph);
    boost::add_edge(FF + b.data.geo_ref.car_offset, EE + b.data.geo_ref.car_offset, ng::Edge(2,5), b.data.geo_ref.graph);
    b.data.geo_ref.ways[2]->edges.push_back(std::make_pair(EE , FF));
    b.data.geo_ref.ways[2]->edges.push_back(std::make_pair(FF , EE));

// F->C
    boost::add_edge(FF , CC , ng::Edge(3,5), b.data.geo_ref.graph);
    boost::add_edge(CC , FF , ng::Edge(3,5), b.data.geo_ref.graph);
    boost::add_edge(FF + b.data.geo_ref.car_offset, CC + b.data.geo_ref.car_offset, ng::Edge(3,5), b.data.geo_ref.graph);
    boost::add_edge(CC + b.data.geo_ref.car_offset, FF + b.data.geo_ref.car_offset, ng::Edge(3,5), b.data.geo_ref.graph);
    b.data.geo_ref.ways[3]->edges.push_back(std::make_pair(FF , CC));
    b.data.geo_ref.ways[3]->edges.push_back(std::make_pair(CC , FF));

// C->B
    boost::add_edge(CC , BB , ng::Edge(4,5), b.data.geo_ref.graph);
    boost::add_edge(BB , CC , ng::Edge(4,5), b.data.geo_ref.graph);
    boost::add_edge(CC + b.data.geo_ref.car_offset, BB + b.data.geo_ref.car_offset, ng::Edge(4,5), b.data.geo_ref.graph);
    boost::add_edge(BB + b.data.geo_ref.car_offset, CC + b.data.geo_ref.car_offset, ng::Edge(4,5), b.data.geo_ref.graph);
    b.data.geo_ref.ways[4]->edges.push_back(std::make_pair(CC , BB));
    b.data.geo_ref.ways[4]->edges.push_back(std::make_pair(BB , CC));

// A->G
    boost::add_edge(AA , GG , ng::Edge(5,5), b.data.geo_ref.graph);
    boost::add_edge(GG , AA , ng::Edge(5,5), b.data.geo_ref.graph);
    boost::add_edge(AA + b.data.geo_ref.bike_offset, GG + b.data.geo_ref.bike_offset, ng::Edge(5,5), b.data.geo_ref.graph);
    boost::add_edge(GG + b.data.geo_ref.bike_offset, AA + b.data.geo_ref.bike_offset, ng::Edge(5,5), b.data.geo_ref.graph);
    b.data.geo_ref.ways[5]->edges.push_back(std::make_pair(AA , GG));
    b.data.geo_ref.ways[5]->edges.push_back(std::make_pair(GG , AA));

// G->H
    boost::add_edge(GG , HH , ng::Edge(6,5), b.data.geo_ref.graph);
    boost::add_edge(HH , GG , ng::Edge(6,5), b.data.geo_ref.graph);
    boost::add_edge(GG + b.data.geo_ref.bike_offset, HH + b.data.geo_ref.bike_offset, ng::Edge(6,5), b.data.geo_ref.graph);
    boost::add_edge(HH + b.data.geo_ref.bike_offset, GG + b.data.geo_ref.bike_offset, ng::Edge(6,5), b.data.geo_ref.graph);
    b.data.geo_ref.ways[6]->edges.push_back(std::make_pair(GG , HH));
    b.data.geo_ref.ways[6]->edges.push_back(std::make_pair(HH , GG));

// H->I
    boost::add_edge(HH , II , ng::Edge(7,5), b.data.geo_ref.graph);
    boost::add_edge(II , HH , ng::Edge(7,5), b.data.geo_ref.graph);
    boost::add_edge(HH + b.data.geo_ref.bike_offset, II + b.data.geo_ref.bike_offset, ng::Edge(7,5), b.data.geo_ref.graph);
    boost::add_edge(II + b.data.geo_ref.bike_offset, HH + b.data.geo_ref.bike_offset, ng::Edge(7,5), b.data.geo_ref.graph);
    b.data.geo_ref.ways[7]->edges.push_back(std::make_pair(HH , II));
    b.data.geo_ref.ways[7]->edges.push_back(std::make_pair(II , HH));

// I->J
    boost::add_edge(II , JJ , ng::Edge(8,5), b.data.geo_ref.graph);
    boost::add_edge(JJ , II , ng::Edge(8,5), b.data.geo_ref.graph);
    boost::add_edge(II + b.data.geo_ref.bike_offset, JJ + b.data.geo_ref.bike_offset, ng::Edge(8,5), b.data.geo_ref.graph);
    boost::add_edge(JJ + b.data.geo_ref.bike_offset, II + b.data.geo_ref.bike_offset, ng::Edge(8,5), b.data.geo_ref.graph);
    b.data.geo_ref.ways[8]->edges.push_back(std::make_pair(II , JJ));
    b.data.geo_ref.ways[8]->edges.push_back(std::make_pair(JJ , II));

// J->K
    boost::add_edge(JJ , KK , ng::Edge(9,5), b.data.geo_ref.graph);
    boost::add_edge(KK , JJ , ng::Edge(9,5), b.data.geo_ref.graph);
    boost::add_edge(JJ + b.data.geo_ref.bike_offset, KK + b.data.geo_ref.bike_offset, ng::Edge(9,5), b.data.geo_ref.graph);
    boost::add_edge(KK + b.data.geo_ref.bike_offset, JJ + b.data.geo_ref.bike_offset, ng::Edge(9,5), b.data.geo_ref.graph);
    b.data.geo_ref.ways[9]->edges.push_back(std::make_pair(JJ , KK));
    b.data.geo_ref.ways[9]->edges.push_back(std::make_pair(KK , JJ));

// K->B
    boost::add_edge(KK , BB , ng::Edge(10,5), b.data.geo_ref.graph);
    boost::add_edge(BB , KK , ng::Edge(10,5), b.data.geo_ref.graph);
    boost::add_edge(KK + b.data.geo_ref.bike_offset, BB + b.data.geo_ref.bike_offset, ng::Edge(10,5), b.data.geo_ref.graph);
    boost::add_edge(BB + b.data.geo_ref.bike_offset, KK + b.data.geo_ref.bike_offset, ng::Edge(10,5), b.data.geo_ref.graph);
    b.data.geo_ref.ways[10]->edges.push_back(std::make_pair(KK , BB));
    b.data.geo_ref.ways[10]->edges.push_back(std::make_pair(BB , KK));

// A->R
    boost::add_edge(AA, RR, ng::Edge(11,10), b.data.geo_ref.graph);
    boost::add_edge(RR, AA, ng::Edge(11,10), b.data.geo_ref.graph);
    b.data.geo_ref.ways[11]->edges.push_back(std::make_pair(AA, RR));
    b.data.geo_ref.ways[11]->edges.push_back(std::make_pair(RR, AA));

// B->S
    boost::add_edge(BB, SS, ng::Edge(12,10), b.data.geo_ref.graph);
    boost::add_edge(SS, BB, ng::Edge(12,10), b.data.geo_ref.graph);
    b.data.geo_ref.ways[12]->edges.push_back(std::make_pair(BB, SS));
    b.data.geo_ref.ways[12]->edges.push_back(std::make_pair(SS, BB));
    boost::add_edge(BB + b.data.geo_ref.bike_offset, SS + b.data.geo_ref.bike_offset, ng::Edge(12,5), b.data.geo_ref.graph);
    boost::add_edge(SS + b.data.geo_ref.bike_offset, BB + b.data.geo_ref.bike_offset, ng::Edge(12,5), b.data.geo_ref.graph);
    boost::add_edge(BB + b.data.geo_ref.car_offset, SS + b.data.geo_ref.car_offset, ng::Edge(12,5), b.data.geo_ref.graph);
    boost::add_edge(SS + b.data.geo_ref.car_offset, BB + b.data.geo_ref.car_offset, ng::Edge(12,5), b.data.geo_ref.graph);

    b.sa("stopA", A.lon(), A.lat());
    b.sa("stopR", R.lon(), R.lat());
    b.vj("A")("stopA", 8*3600 +10*60, 8*3600 + 11 * 60)("stopR", 8*3600 + 20 * 60 ,8*3600 + 21*60);
    b.generate_dummy_basis();
    b.data.pt_data.index();
    b.data.build_raptor();
    b.data.build_uri();
    b.data.build_proximity_list();
    b.data.meta.production_date = boost::gregorian::date_period(boost::gregorian::date(2012,06,14), boost::gregorian::days(7));
    streetnetwork::StreetNetwork sn_worker(b.data.geo_ref);

    RAPTOR raptor(b.data);


    type::EntryPoint origin("coord:"+boost::lexical_cast<std::string>(S.lon())+":"+boost::lexical_cast<std::string>(S.lat()));
    origin.streetnetwork_params.mode = navitia::type::Mode_e::Walking;
    origin.streetnetwork_params.offset = 0;
    origin.streetnetwork_params.distance = 15;
    type::EntryPoint destination("coord:"+boost::lexical_cast<std::string>(R.lon())+":"+boost::lexical_cast<std::string>(R.lat()));
    destination.streetnetwork_params.mode = navitia::type::Mode_e::Walking;
    destination.streetnetwork_params.offset = 0;
    destination.streetnetwork_params.distance = 5;
    // On met les horaires dans le desordre pour voir s'ils sont bien triÃ© comme attendu
    std::vector<std::string> datetimes({"20120614T080000", "20120614T090000"});
    std::vector<std::string> forbidden;

    pbnavitia::Response resp = make_response(raptor, origin, destination, datetimes, true, 1.38, 1000, false, forbidden, sn_worker);

// Marche à pied
    BOOST_REQUIRE_EQUAL(resp.journeys_size(), 3);
    pbnavitia::Journey journey = resp.journeys(0);
    BOOST_REQUIRE_EQUAL(journey.sections_size(), 1);
    pbnavitia::Section section = journey.sections(0);
    BOOST_REQUIRE_EQUAL(section.type(), pbnavitia::SectionType::STREET_NETWORK);
    BOOST_REQUIRE_EQUAL(section.street_network().coordinates_size(), 5);
    BOOST_REQUIRE_EQUAL(section.street_network().length(), 10);
    BOOST_REQUIRE_EQUAL(section.street_network().mode(), pbnavitia::StreetNetworkMode::Walking);
    BOOST_REQUIRE_EQUAL(section.street_network().path_items_size(), 1);
    pbnavitia::PathItem pathitem = section.street_network().path_items(0);
    BOOST_REQUIRE_EQUAL(pathitem.name(), "rue ab");
    BOOST_REQUIRE_EQUAL(pathitem.length(), 10);
// vélo
    origin.streetnetwork_params.mode = navitia::type::Mode_e::Bike;
    origin.streetnetwork_params.offset = b.data.geo_ref.bike_offset;
    origin.streetnetwork_params.speed = 13;
    origin.streetnetwork_params.distance = S.distance_to(B) + B.distance_to(K) + J.distance_to(I) + H.distance_to(G) + G.distance_to(A) + 1;
    destination.streetnetwork_params.mode = navitia::type::Mode_e::Bike;
    destination.streetnetwork_params.offset = b.data.geo_ref.bike_offset;
    destination.streetnetwork_params.distance = 5;
    resp = make_response(raptor, origin, destination, datetimes, true, 1.38, 1000, false, forbidden, sn_worker);

    BOOST_REQUIRE_EQUAL(resp.response_type(), pbnavitia::ITINERARY_FOUND);
    BOOST_REQUIRE_EQUAL(resp.journeys_size(), 3);
    journey = resp.journeys(0);
    BOOST_REQUIRE_EQUAL(journey.sections_size(), 1);
    section = journey.sections(0);
    BOOST_REQUIRE_EQUAL(section.type(), pbnavitia::SectionType::STREET_NETWORK);
    BOOST_REQUIRE_EQUAL(section.origin().address().name(), "rue kb");
    BOOST_REQUIRE_EQUAL(section.destination().address().name(), "rue ag");
    BOOST_REQUIRE_EQUAL(section.street_network().coordinates_size(), 10);
    BOOST_REQUIRE_EQUAL(section.street_network().mode(), pbnavitia::StreetNetworkMode::Bike);
    BOOST_REQUIRE_EQUAL(section.street_network().length(), 30);
    BOOST_REQUIRE_EQUAL(section.street_network().path_items_size(), 6);

    pathitem = section.street_network().path_items(0);
    BOOST_REQUIRE_EQUAL(pathitem.name(), "rue kb");
    pathitem = section.street_network().path_items(1);
    BOOST_REQUIRE_EQUAL(pathitem.name(), "rue jk");
    pathitem = section.street_network().path_items(2);
    BOOST_REQUIRE_EQUAL(pathitem.name(), "rue ij");
    pathitem = section.street_network().path_items(3);
    BOOST_REQUIRE_EQUAL(pathitem.name(), "rue hi");
    pathitem = section.street_network().path_items(4);
    BOOST_REQUIRE_EQUAL(pathitem.name(), "rue gh");
    pathitem = section.street_network().path_items(5);
    BOOST_REQUIRE_EQUAL(pathitem.name(), "rue ag");

// Voiture
    origin.streetnetwork_params.mode = navitia::type::Mode_e::Car;
    origin.streetnetwork_params.offset = b.data.geo_ref.car_offset;
    origin.streetnetwork_params.speed = 13;
    origin.streetnetwork_params.distance = S.distance_to(B) + B.distance_to(C) + C.distance_to(F) + F.distance_to(E) + E.distance_to(A) + 1;
    destination.streetnetwork_params.mode = navitia::type::Mode_e::Car;
    destination.streetnetwork_params.offset = b.data.geo_ref.car_offset;
    destination.streetnetwork_params.distance = 5;
    resp = make_response(raptor, origin, destination, datetimes, true, 1.38, 1000, false, forbidden, sn_worker);

    BOOST_REQUIRE_EQUAL(resp.response_type(), pbnavitia::ITINERARY_FOUND);
    BOOST_REQUIRE_EQUAL(resp.journeys_size(), 3);
    journey = resp.journeys(0);
    BOOST_REQUIRE_EQUAL(journey.sections_size(), 1);
    section = journey.sections(0);
    BOOST_REQUIRE_EQUAL(section.type(), pbnavitia::SectionType::STREET_NETWORK);
    BOOST_REQUIRE_EQUAL(section.origin().address().name(), "rue cb");
    BOOST_REQUIRE_EQUAL(section.destination().address().name(), "rue ae");
    BOOST_REQUIRE_EQUAL(section.street_network().coordinates_size(), 8);
    BOOST_REQUIRE_EQUAL(section.street_network().mode(), pbnavitia::StreetNetworkMode::Car);
    BOOST_REQUIRE_EQUAL(section.street_network().length(), 20);
    BOOST_REQUIRE_EQUAL(section.street_network().path_items_size(), 4);
    pathitem = section.street_network().path_items(0);
    BOOST_REQUIRE_EQUAL(pathitem.name(), "rue cb");
    pathitem = section.street_network().path_items(1);
    BOOST_REQUIRE_EQUAL(pathitem.name(), "rue fc");
    pathitem = section.street_network().path_items(2);
    BOOST_REQUIRE_EQUAL(pathitem.name(), "rue ef");

}
