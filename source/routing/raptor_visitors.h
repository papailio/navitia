#pragma once

#include <boost/range/iterator_range_core.hpp>

namespace navitia { namespace routing {
struct raptor_visitor {
    inline bool better_or_equal(const DateTime& a, const DateTime& current_dt, const type::StopTime& st) const {
        return a <= st.section_end_date(DateTimeUtils::date(current_dt), clockwise());
    }

    inline boost::iterator_range<std::vector<dataRAPTOR::JppsFromJp::Jpp>::const_iterator>
    jpps_from_order(const dataRAPTOR::JppsFromJp& jpps_from_jp, JpIdx jp_idx, uint16_t jpp_order) const {
        const auto& jpps = jpps_from_jp[jp_idx];
        return boost::make_iterator_range(jpps.begin() + jpp_order, jpps.end());
    }

    typedef std::vector<type::StopTime>::const_iterator stop_time_iterator;
    boost::iterator_range<stop_time_iterator>
    st_range(const type::StopTime& st) const {
        const type::JourneyPatternPoint* jpp = st.journey_pattern_point;
        const type::VehicleJourney* vj = st.vehicle_journey;
        return boost::make_iterator_range(vj->stop_time_list.begin() + jpp->order,
                                          vj->stop_time_list.end());
    }

    template<typename T1, typename T2> inline bool comp(const T1& a, const T2& b) const {
        return a < b;
    }
    // better or equal
    template<typename T1, typename T2> inline bool be(const T1& a, const T2& b) const {
        return ! comp(b, a);
    }

    template<typename T1, typename T2> inline auto combine(const T1& a, const T2& b) const -> decltype(a+b) {
        return a + b;
    }

    inline
    const type::VehicleJourney* get_extension_vj(const type::VehicleJourney* vj) const {
       return vj->next_vj;
    }

    inline
    const type::JourneyPatternPoint* get_last_jpp(const type::VehicleJourney* vj) const {
       if(vj->prev_vj) {
            return vj->prev_vj->journey_pattern->journey_pattern_point_list.back();
       } else {
           return nullptr;
       }
    }

    inline
    boost::iterator_range<std::vector<type::StopTime>::const_iterator>
    stop_time_list(const type::VehicleJourney* vj) const {
        return boost::make_iterator_range(vj->stop_time_list.begin(), vj->stop_time_list.end());
    }

    constexpr bool clockwise() const{return true;}
    constexpr int init_queue_item() const{return std::numeric_limits<int>::max();}
    constexpr DateTime worst_datetime() const{return DateTimeUtils::inf;}
};


struct raptor_reverse_visitor {
    inline bool better_or_equal(const DateTime &a, const DateTime &current_dt, const type::StopTime& st) const {
        return a >= st.section_end_date(DateTimeUtils::date(current_dt), clockwise());
    }

    inline boost::iterator_range<std::vector<dataRAPTOR::JppsFromJp::Jpp>::const_reverse_iterator>
    jpps_from_order(const dataRAPTOR::JppsFromJp& jpps_from_jp, JpIdx jp_idx, uint16_t jpp_order) const {
        const auto& jpps = jpps_from_jp[jp_idx];
        return boost::make_iterator_range(jpps.rbegin() + jpps.size() - jpp_order - 1, jpps.rend());
    }

    typedef std::vector<type::StopTime>::const_reverse_iterator stop_time_iterator;
    inline boost::iterator_range<stop_time_iterator>
    st_range(const type::StopTime& st) const {
        const type::JourneyPatternPoint* jpp = st.journey_pattern_point;
        const type::VehicleJourney* vj = st.vehicle_journey;
        return boost::make_iterator_range(
            vj->stop_time_list.rbegin() + vj->stop_time_list.size() - jpp->order - 1,
            vj->stop_time_list.rend());
    }

    template<typename T1, typename T2> inline bool comp(const T1& a, const T2& b) const {
        return a > b;
    }
    // better or equal
    template<typename T1, typename T2> inline bool be(const T1& a, const T2& b) const {
        return ! comp(b, a);
    }

    template<typename T1, typename T2> inline auto combine(const T1& a, const T2& b) const -> decltype(a-b) {
        return a - b;
    }

    inline
    const type::VehicleJourney* get_extension_vj(const type::VehicleJourney* vj) const {
       return vj->prev_vj;
    }

    inline
    const type::JourneyPatternPoint* get_last_jpp(const type::VehicleJourney* vj) const {
        if(vj->next_vj != nullptr) {
            return vj->next_vj->journey_pattern->journey_pattern_point_list.front();
        } else {
            return nullptr;
        }
    }

    inline
    boost::iterator_range<std::vector<type::StopTime>::const_reverse_iterator>
    stop_time_list(const type::VehicleJourney* vj) const {
        return boost::make_iterator_range(vj->stop_time_list.rbegin(), vj->stop_time_list.rend());
    }

    constexpr bool clockwise() const{return false;}
    constexpr int init_queue_item() const{return -1;}
    constexpr DateTime worst_datetime() const{return DateTimeUtils::min;}
};

}}
