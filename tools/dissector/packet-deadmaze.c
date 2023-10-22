#include <stdbool.h>

#include <config.h>
#include <epan/packet.h>

static int proto_deadmaze = -1;

static int hf_deadmaze_data = -1;
static int hf_deadmaze_length = -1;
static int hf_deadmaze_opcode = -1;
static int hf_deadmaze_sequence = -1;

static int ett_deadmaze = -1;

static const value_string opcode_names[] = {
    {0x1a1a, "Keep Alive"},
    {0x1a32, "Disconnect"},
    {0x1c06, "Ping"},
    {0, NULL}
};

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

const gchar plugin_version[] = "0.0.0";
const int plugin_want_major = VERSION_MAJOR;
const int plugin_want_minor = VERSION_MINOR;

static bool read_varint(guint32 *result, tvbuff_t *tvb, guint *offset) {
    guint shift = 0;
    guint32 r = 0;
    const guint length = tvb_reported_length(tvb);

    while (*offset < length && shift <= 35) {
        const guint8 b = tvb_get_guint8(tvb, *offset);
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
    return pinfo->destport % 1000 == 801 && s >=11 && s <=13;
}

static void dissect_deadmaze_packet(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, guint *offset, guint32 size) {
    col_set_str(pinfo->cinfo, COL_PROTOCOL, "DeadMaze");
    col_clear(pinfo->cinfo, COL_INFO);

    // Skipping client sequence
    if (is_to_server(pinfo)) {
        proto_tree_add_item(tree, hf_deadmaze_sequence, tvb, *offset, 1, ENC_BIG_ENDIAN);
       *offset += 1;
    }

    proto_tree_add_item(tree, hf_deadmaze_opcode, tvb, *offset, 2, ENC_BIG_ENDIAN);
    *offset += 2;

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

void proto_register_deadmaze(void) {
    proto_deadmaze = proto_register_protocol("DeadMaze Protocol", "DeadMaze", "deadmaze");

    proto_register_field_array(proto_deadmaze, hf, array_length(hf));
    proto_register_subtree_array(ett, array_length(ett));
}

void proto_reg_handoff_deadmaze(void) {
    static dissector_handle_t deadmaze_handle;

    deadmaze_handle = create_dissector_handle(dissect_deadmaze, proto_deadmaze);
    // TODO(Mord3rca): Find a way to apply filter (tcp.port % 1000 == 801)
    dissector_add_uint("tcp.port", 11801, deadmaze_handle);
    dissector_add_uint("tcp.port", 12801, deadmaze_handle);
    dissector_add_uint("tcp.port", 13801, deadmaze_handle);
}

void plugin_register(void) {
    static proto_plugin plug;

    plug.register_protoinfo = proto_register_deadmaze;
    plug.register_handoff = proto_reg_handoff_deadmaze;
    proto_register_plugin(&plug);
}
