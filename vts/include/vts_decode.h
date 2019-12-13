#ifndef VTS_DECODE_H
#define VTS_DECODE_H

//std/stl
#include <cstdint>
#include <vector>
#include <functional> // std::unary_function

namespace vts
{
namespace decode
{

    struct endian_swap32 : public std::unary_function<uint32_t, uint32_t>
    {
        endian_swap32(){};
        uint32_t operator() (const uint32_t& x_)
        {
            uint32_t x = x_;
            return
            ((x>>24) & 0x000000FF) |
            ((x<<8)  & 0x00FF0000) |
            ((x>>8)  & 0x0000FF00) |
            ((x<<24) & 0xFF000000);
        }
    };

namespace vmm
{
    class Header
    {
        public :
            Header()
            {
                clear();
            }

            void clear()
            {
                // art info
                m_art_trigger = 0x0;
                m_art_valid_flag = 0x0;
                m_art_address = 0x0;

                // vmm
                m_frame = 0x0;
                m_trigger_counter = 0x0;
                m_vmm_id = 0x0;
                m_overflow_bit = 0x0;
                m_parity_bit = 0x0;
                m_orbit_counter = 0x0;
                m_bcid = 0x0;
                m_gray_decoded_bcid = 0x0;
            }

            void set_art_trigger(uint32_t x) { m_art_trigger = x; }
            void set_art_valid_flag(uint32_t x) { m_art_valid_flag = x; }
            void set_art_address(uint32_t x) { m_art_address = x; }

            void set_frame(uint32_t x) { m_frame = x; }
            void set_trigger_counter(uint32_t x) { m_trigger_counter = x; }
            void set_vmm_id(uint32_t x) { m_vmm_id = x; }
            void set_overflow_bit(uint32_t x) { m_overflow_bit = x; }
            void set_parity_bit(uint32_t x) { m_parity_bit = x; }
            void set_orbit_counter(uint32_t x) { m_orbit_counter = x; }
            void set_bcid(uint32_t x) { m_bcid = x; }
            void set_gray_decoded_bcid(uint32_t x) { m_gray_decoded_bcid = x; }

            const uint32_t& art_trigger() const { return m_art_trigger; }
            const uint32_t& art_valid_flag() const { return m_art_valid_flag; }
            const uint32_t& art_address() const { return m_art_address; }

            const uint32_t& frame() const { return m_frame; }
            const uint32_t& trigger_counter() const { return m_trigger_counter; }
            const uint32_t& vmm_id() const { return m_vmm_id; }
            const uint32_t& overflow_bit() const { return m_overflow_bit; }
            const uint32_t& parity_bit() const { return m_parity_bit; }
            const uint32_t& orbit_counter() const { return m_orbit_counter; }
            const uint32_t& bcid() const { return m_bcid; }
            const uint32_t& gray_decoded_bcid() const { return m_gray_decoded_bcid; }

        private :

            // art information
            uint32_t m_art_trigger;
            uint32_t m_art_valid_flag;
            uint32_t m_art_address;

            // vmm info
            uint32_t m_frame;
            uint32_t m_trigger_counter;
            uint32_t m_vmm_id;
            uint32_t m_overflow_bit;
            uint32_t m_parity_bit;
            uint32_t m_orbit_counter;
            uint32_t m_bcid;
            uint32_t m_gray_decoded_bcid;
        

    }; // struct header

    class Sample
    {
        public :
            Sample() { clear(); }
            virtual ~Sample(){};

            void clear()
            {
                m_header.clear();
            }

            void set_header(Header h) { m_header = h; }
            const Header& header() const { return m_header; }

            void set_frame(uint32_t x) { m_frame = x; }
            void set_parity_bit(uint32_t x) { m_parity_bit = x; }
            void set_truncation_bit(uint32_t x) { m_truncation_bit = x; }
            void set_channel(uint32_t x) { m_channel = x; }
            void set_pdo(uint32_t x) { m_pdo = x; }
            void set_tdo(uint32_t x) { m_tdo = x; }
            void set_is_neighbor(uint32_t x) { m_is_neighbor = x; }
            void set_rel_bcid(uint32_t x) { m_rel_bcid = x; }

            const uint32_t& frame() const { return m_frame; }
            const uint32_t& parity_bit() const { return m_parity_bit; }
            const uint32_t& truncation_bit() const { return m_truncation_bit; }
            const uint32_t& channel() const { return m_channel; }
            const uint32_t& pdo() const { return m_pdo; }
            const uint32_t& tdo() const { return m_tdo; }
            const uint32_t& is_neighbor() const { return m_is_neighbor; }
            const uint32_t& rel_bcid() const { return m_rel_bcid; }

        private :

            Header m_header;
            uint32_t m_frame;
            uint32_t m_parity_bit;
            uint32_t m_truncation_bit;
            uint32_t m_channel;
            uint32_t m_pdo;
            uint32_t m_tdo;
            uint32_t m_is_neighbor;
            uint32_t m_rel_bcid;
            
    };

    std::vector<vts::decode::vmm::Sample> decode(std::vector<uint32_t> data);
    

} // namespace vmm

namespace xadc
{
    class Sample
    {
        public :
            Sample() { clear(); }
            virtual ~Sample(){};

            void clear()
            {
                m_vmm_id = 0x0;
                m_sample = 0x0;
            }

            void set_vmm_id(uint32_t x) { m_vmm_id = x; }
            const uint32_t& vmm_id() const { return m_vmm_id; }

            void set_sample(uint32_t x) { m_sample = x; }
            const uint32_t& sample() const { return m_sample; }

        private :
            uint32_t m_vmm_id;
            uint32_t m_sample;
    }; // class Sample

    std::vector<vts::decode::xadc::Sample> decode(std::vector<uint32_t> data);

} // namespace xadc

} // namespace daq
} // namespace vts

#endif
