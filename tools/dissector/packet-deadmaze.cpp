#include <arpa/inet.h>

#include <config.h>
#include <epan/packet.h>
#include <ws_version.h>

#include "openmeat/crypto"
#include "openmeat/opcodes"

static int proto_deadmaze = -1;

static int hf_deadmaze_data = -1;
static int hf_deadmaze_length = -1;
static int hf_deadmaze_opcode = -1;
static int hf_deadmaze_sequence = -1;

static int ett_deadmaze = -1;

#define OPCODE_TO_VSTRING(val) \
    {static_cast<uint32_t>(Openmeat::Network::opcode_t::val), #val}

static const value_string opcode_names[] = {
    OPCODE_TO_VSTRING(LOGIN),
    OPCODE_TO_VSTRING(DISCONNECT),
    OPCODE_TO_VSTRING(KEEP_ALIVE),
    OPCODE_TO_VSTRING(PING),
    OPCODE_TO_VSTRING(COMMUNITY),
    OPCODE_TO_VSTRING(MOVE),
    OPCODE_TO_VSTRING(PERK),
    OPCODE_TO_VSTRING(USE),
    OPCODE_TO_VSTRING(ATTACK),
    OPCODE_TO_VSTRING(USE_OBJECT),
    OPCODE_TO_VSTRING(SWAP_WEAPON),
    OPCODE_TO_VSTRING(CRAFT),
    OPCODE_TO_VSTRING(TRAVEL),
    OPCODE_TO_VSTRING(PARRY),
    {0, NULL}
};

#undef OPCODE_TO_VSTRING

static hf_register_info hf[] = {
    { &hf_deadmaze_length, {
        "Length", "deadmaze.length", FT_UINT16, BASE_DEC,
        NULL, 0x0, "Packet length", HFILL
    }},
    { &hf_deadmaze_opcode, {
        "Opcode", "deadmaze.opcode", FT_UINT16, BASE_HEX,
        VALS(opcode_names), 0x0, "Packet opcode", HFILL
    }},
    { &hf_deadmaze_data, {
       "Data", "deadmaze.data", FT_BYTES, BASE_NONE,
       NULL, 0x0, "Packet data", HFILL
    }},
    { &hf_deadmaze_sequence, {
       "Seq", "deadmaze.seq", FT_UINT8, BASE_HEX,
       NULL, 0x0, "Client sequence", HFILL
    }},
};

static int *ett[] = {
    &ett_deadmaze
};

static bool read_varint(guint32 *result, tvbuff_t *tvb, guint *offset) {
    guint shift = 0;
    guint32 r = 0;
    const unsigned int length = tvb_reported_length(tvb);

    while (*offset < length && shift <= 35) {
#if WIRESHARK_VERSION_MAJOR >= 4 && WIRESHARK_VERSION_MINOR >=6
        const uint8_t b = tvb_get_uint8(tvb, *offset);
#else
        const guint8 b = tvb_get_guint8(tvb, *offset);
#endif
        r |= ((b & 0x7f) << shift);
        *offset += 1;
        shift += 7;
        if ((b & 0x80) == 0) {
            *result = r;
            return true;
        }
    }

    return false;
}

static bool is_to_server(const packet_info *pinfo) {
    int s = pinfo->destport / 1000;
    return pinfo->destport % 1000 == 801 && s >=11 && s <=14;
}

#ifdef DISSECTOR_IDENT_KEY
static inline void _apply_ntohl(std::int32_t arr[], std::size_t len) {
    for (std::size_t i=0; i < len; i++)
        arr[i] = ntohl(arr[i]);
}

constexpr Openmeat::Crypto::Key ikey(DISSECTOR_IDENT_KEY);
static void _unscramble(unsigned char *edata) {
        std::size_t len = ntohs(*reinterpret_cast<std::uint16_t*>(edata));
        std::int32_t *arr = reinterpret_cast<std::int32_t*>(edata+sizeof(std::uint16_t));

        _apply_ntohl(arr, len);
        Openmeat::Crypto::unscramble(ikey, arr, len);
        _apply_ntohl(arr, len);
}
#endif

#ifdef DISSECTOR_MSG_KEY
constexpr Openmeat::Crypto::Key mkey(DISSECTOR_MSG_KEY);
#endif

static void dissect_deadmaze_packet(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, guint *offset, guint32 size) {
    unsigned char seq;
    Openmeat::Network::opcode_t opcode;
    bool from_client = is_to_server(pinfo);

    col_set_str(pinfo->cinfo, COL_PROTOCOL, "DeadMaze");
    col_clear(pinfo->cinfo, COL_INFO);

    // Skipping client sequence
    if (from_client) {
#if WIRESHARK_VERSION_MAJOR >= 4 && WIRESHARK_VERSION_MINOR >=6
        seq = tvb_get_uint8(tvb, *offset);
#else
        seq = tvb_get_guint8(tvb, *offset);
#endif
        proto_tree_add_item(tree, hf_deadmaze_sequence, tvb, *offset, 1, ENC_BIG_ENDIAN);
       *offset += 1;
    }

    opcode = static_cast<Openmeat::Network::opcode_t>(tvb_get_ntohs(tvb, *offset));
    proto_tree_add_item(tree, hf_deadmaze_opcode, tvb, *offset, 2, ENC_BIG_ENDIAN);
    *offset += 2;

    if ((size-2) <= 0)
        return;

    if (from_client) {
        switch (opcode) {
#ifdef DISSECTOR_IDENT_KEY
        case Openmeat::Network::opcode_t::LOGIN:
        {
            unsigned char *edata = reinterpret_cast<unsigned char*>(tvb_memdup(pinfo->pool, tvb, *offset, size-2));
            _unscramble(edata);
            tvbuff_t *next_tvb = tvb_new_child_real_data(tvb, edata, size-2, size-2);
            add_new_data_source(pinfo, next_tvb, "Unscrambled Data");
            break;
        }
#endif
#ifdef DISSECTOR_MSG_KEY
        case Openmeat::Network::opcode_t::COMMUNITY:
        {
            unsigned char *cdata = reinterpret_cast<unsigned char*>(tvb_memdup(pinfo->pool, tvb, *offset, size-2));
            Openmeat::Crypto::xoring(mkey, seq+1, cdata, size-2);
            tvbuff_t *next_tvb = tvb_new_child_real_data(tvb, cdata, size-2, size-2);
            add_new_data_source(pinfo, next_tvb, "XORed Data");
            break;
        }
#endif
        default:
            ;;
        }
    }

    proto_tree_add_item(tree, hf_deadmaze_data, tvb, *offset, size-2, ENC_BIG_ENDIAN);
    *offset += size - 2;
}

static int dissect_deadmaze(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_) {
    guint offset = 0;

    while (offset < tvb_reported_length(tvb)) {
        gint available = tvb_reported_length_remaining(tvb, offset);
        guint32 size = 0;
        const guint offset_start = offset;

        if (!read_varint(&size, tvb, &offset)) {
            pinfo->desegment_offset = offset;
            pinfo->desegment_len = DESEGMENT_ONE_MORE_SEGMENT;
            return offset + available;
        }

        if (size > available) {
            pinfo->desegment_offset = offset;
            pinfo->desegment_len = size - available;
            return offset + available;
        }

        proto_item *item = proto_tree_add_item(tree, proto_deadmaze, tvb, 0, -1, ENC_NA);
        proto_tree *deadmaze_tree = proto_item_add_subtree(item, ett_deadmaze);

        proto_tree_add_uint(deadmaze_tree, hf_deadmaze_length, tvb, offset_start, (offset - offset_start), size);
        dissect_deadmaze_packet(tvb, pinfo, deadmaze_tree, &offset, size);
    }

    return tvb_captured_length(tvb);
}

extern "C" const gchar plugin_version[] = OPENMEAT_VERSION;
extern "C" const int plugin_want_major = VERSION_MAJOR;
extern "C" const int plugin_want_minor = VERSION_MINOR;

extern "C" void proto_register_deadmaze(void) {
    proto_deadmaze = proto_register_protocol("DeadMaze Protocol", "DeadMaze", "deadmaze");

    proto_register_field_array(proto_deadmaze, hf, array_length(hf));
    proto_register_subtree_array(ett, array_length(ett));
}

extern "C" void proto_reg_handoff_deadmaze(void) {
    static dissector_handle_t deadmaze_handle;

    deadmaze_handle = create_dissector_handle(dissect_deadmaze, proto_deadmaze);
    // TODO(Mord3rca): Find a way to apply filter (tcp.port % 1000 == 801)
    dissector_add_uint("tcp.port", 11801, deadmaze_handle);
    dissector_add_uint("tcp.port", 12801, deadmaze_handle);
    dissector_add_uint("tcp.port", 13801, deadmaze_handle);
    dissector_add_uint("tcp.port", 14801, deadmaze_handle);
}

extern "C" void plugin_register(void) {
    static proto_plugin plug;

    plug.register_protoinfo = proto_register_deadmaze;
    plug.register_handoff = proto_reg_handoff_deadmaze;
    proto_register_plugin(&plug);
}
