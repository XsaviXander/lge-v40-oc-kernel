#define pr_fmt(fmt)	"[Display][sw43408a-ops:%s:%d] " fmt, __func__, __LINE__

#include "dsi_panel.h"
#include "lge_ddic_ops_helper.h"
#include "cm/lge_color_manager.h"

#define ADDR_PTLAR 0x30
#define ADDR_PLTAC 0x31
#define ADDR_RDDISPM 0x3F
#define ADDR_ERR_DETECT 0x9F
#define ADDR_WRIECTL 0x55
#define ADDR_PWRCTL3 0xC3

#define WORDS_TO_BYTE_ARRAY(w1, w2, b) do {\
		b[0] = WORD_UPPER_BYTE(w1);\
		b[1] = WORD_LOWER_BYTE(w1);\
		b[2] = WORD_UPPER_BYTE(w2);\
		b[3] = WORD_LOWER_BYTE(w2);\
} while(0)

extern int lge_ddic_dsi_panel_tx_cmd_set(struct dsi_panel *panel,
				enum lge_ddic_dsi_cmd_set_type type);
extern int lge_mdss_dsi_panel_cmd_read(struct dsi_panel *panel,
					u8 cmd, int cnt, char* ret_buf);
extern int lge_backlight_device_update_status(struct backlight_device *bd);

const struct drs_res_info sw43408a_res[3] = {
	{"qhd", 0, 1440, 3120},
	{"fhd", 1, 1080, 2340},
	{"hd", 3, 720, 1560},
};

#define IDX_DG_CTRL1 1
#define REG_DG_CTRL1 0xEC
#define NUM_DG_CTRL1 32
#define START_DG_CTRL1 2
#define NUM_BLUE_LOWER_CTRL1 4

#define IDX_DG_CTRL2 2
#define REG_DG_CTRL2 0xED
#define NUM_DG_CTRL2 5
#define START_DG_CTRL2 1
#define NUM_BLUE_LOWER_CTRL2 5

#define OFFSET_DG_UPPER 3
#define OFFSET_DG_LOWER 9

#define STEP_DG_PRESET 5
#define NUM_DG_PRESET  24

#define PRESET_SETP0_INDEX 0
#define PRESET_SETP1_INDEX 5
#define PRESET_SETP2_INDEX 12

enum {
	screen_mode_auto = 0,
	screen_mode_cinema,
	screen_mode_photos,
	screen_mode_web,
	screen_mode_sports,
	screen_mode_game,
	screen_mode_expert = 10,
};

static int rgb_preset[STEP_DG_PRESET][RGB_ALL] = {
	{PRESET_SETP2_INDEX, PRESET_SETP0_INDEX, PRESET_SETP2_INDEX},
	{PRESET_SETP1_INDEX, PRESET_SETP0_INDEX, PRESET_SETP1_INDEX},
	{PRESET_SETP0_INDEX, PRESET_SETP0_INDEX, PRESET_SETP0_INDEX},
	{PRESET_SETP0_INDEX, PRESET_SETP1_INDEX, PRESET_SETP0_INDEX},
	{PRESET_SETP0_INDEX, PRESET_SETP2_INDEX, PRESET_SETP0_INDEX}
};

static char dg_ctrl1_values[NUM_DG_PRESET][OFFSET_DG_UPPER] = {
	{0x03, 0x05, 0xAF},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B},
	{0x03, 0x01, 0x6B}
};

static char dg_ctrl2_values[NUM_DG_PRESET][OFFSET_DG_LOWER] = {
	{0x00, 0xFF, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80},
	{0x00, 0xFB, 0x7F, 0xFF, 0x7E, 0xFE, 0x7E, 0xFD, 0x7D},
	{0x00, 0xF8, 0x7F, 0xFE, 0x7D, 0xFC, 0x7C, 0xFB, 0x7A},
	{0x00, 0xF5, 0x7E, 0xFD, 0x7C, 0xFB, 0x7A, 0xF9, 0x77},
	{0x00, 0xF2, 0x7E, 0xFC, 0x7B, 0xF9, 0x78, 0xF6, 0x75},
	{0x00, 0xF0, 0x7E, 0xFC, 0x7A, 0xF8, 0x76, 0xF4, 0x72},
	{0x00, 0xEC, 0x7D, 0xFB, 0x79, 0xF6, 0x74, 0xF2, 0x6F},
	{0x00, 0xE9, 0x7D, 0xFA, 0x77, 0xF5, 0x72, 0xEF, 0x6D},
	{0x00, 0xE6, 0x7C, 0xF9, 0x76, 0xF3, 0x70, 0xED, 0x6A},
	{0x00, 0xE3, 0x7C, 0xF9, 0x75, 0xF2, 0x6E, 0xEB, 0x67},
	{0x00, 0xE0, 0x7C, 0xF8, 0x74, 0xF0, 0x6C, 0xE8, 0x64},
	{0x00, 0xDD, 0x7B, 0xF7, 0x73, 0xEF, 0x6A, 0xE6, 0x62},
	{0x00, 0xDA, 0x7B, 0xF6, 0x72, 0xED, 0x68, 0xE4, 0x5F},
	{0x00, 0xD7, 0x7B, 0xF6, 0x71, 0xEC, 0x67, 0xE2, 0x5D},
	{0x00, 0xD4, 0x7A, 0xF5, 0x6F, 0xEA, 0x65, 0xDF, 0x5A},
	{0x00, 0xD1, 0x7A, 0xF4, 0x6E, 0xE9, 0x63, 0xDD, 0x58},
	{0x00, 0xCE, 0x79, 0xF3, 0x6D, 0xE7, 0x61, 0xDB, 0x55},
	{0x00, 0xCB, 0x79, 0xF3, 0x6C, 0xE6, 0x60, 0xD9, 0x53},
	{0x00, 0xC9, 0x79, 0xF2, 0x6B, 0xE4, 0x5E, 0xD7, 0x50},
	{0x00, 0xC6, 0x78, 0xF1, 0x6A, 0xE3, 0x5C, 0xD5, 0x4E},
	{0x00, 0xC3, 0x78, 0xF1, 0x69, 0xE2, 0x5A, 0xD3, 0x4B},
	{0x00, 0xC0, 0x78, 0xF0, 0x68, 0xE0, 0x58, 0xD1, 0x49},
	{0x00, 0xBD, 0x77, 0xEF, 0x67, 0xDF, 0x57, 0xCF, 0x46},
	{0x00, 0xBA, 0x77, 0xEE, 0x66, 0xDD, 0x55, 0xCC, 0x44}
};

#define NUM_SAT_CTRL 5
#define OFFSET_SAT_CTRL 32
#define REG_SAT_CTRL1 0xCC
#define REG_SAT_CTRL2 0xCD

#define NUM_HUE_CTRL 5
#define OFFSET_HUE_CTRL1 32
#define OFFSET_HUE_CTRL2 5
#define REG_HUE_CTRL1 0xCE
#define REG_HUE_CTRL2 0xCF

#define NUM_SHA_CTRL 5
#define OFFSET_SHA_CTRL 30
#define REG_SHA_CTRL 0xDD

static char saturation_ctrl1_values[NUM_SAT_CTRL][OFFSET_SAT_CTRL] = {
	{0x88, 0x0A, 0x4B, 0x6C, 0xFF, 0x46, 0x4D, 0x4D, 0x66, 0x52, 0x5A, 0x6C, 0x62, 0x5F, 0x58, 0x4D, 0x61, 0x51, 0x4D, 0x4D, 0x53, 0x60, 0x52, 0x6C, 0x62, 0x5E, 0x55, 0x4D, 0x62, 0x62, 0x4A, 0x56},
	{0x88, 0x0A, 0x4B, 0x6C, 0xFF, 0x4F, 0x56, 0x56, 0x73, 0x5D, 0x65, 0x7A, 0x6E, 0x6B, 0x63, 0x56, 0x6D, 0x5B, 0x56, 0x56, 0x5E, 0x6C, 0x5D, 0x7A, 0x6E, 0x69, 0x5F, 0x56, 0x6E, 0x6E, 0x53, 0x61},
	{0x88, 0x0A, 0x4B, 0x6C, 0xFF, 0x58, 0x60, 0x60, 0x80, 0x67, 0x70, 0x87, 0x7A, 0x77, 0x6E, 0x60, 0x79, 0x65, 0x60, 0x60, 0x68, 0x78, 0x67, 0x87, 0x7A, 0x75, 0x6A, 0x60, 0x7A, 0x7A, 0x5C, 0x6C},
	{0x88, 0x0A, 0x4B, 0x6C, 0xFF, 0x61, 0x6A, 0x6A, 0x8D, 0x71, 0x7B, 0x95, 0x86, 0x83, 0x79, 0x6A, 0x85, 0x6F, 0x6A, 0x6A, 0x72, 0x84, 0x71, 0x95, 0x86, 0x81, 0x75, 0x6A, 0x86, 0x7A, 0x5C, 0x6C},
	{0x88, 0x0A, 0x4B, 0x6C, 0xFF, 0x6A, 0x73, 0x73, 0x9A, 0x7C, 0x86, 0xA2, 0x92, 0x8F, 0x84, 0x73, 0x91, 0x79, 0x73, 0x73, 0x7D, 0x90, 0x7C, 0xA2, 0x92, 0x8C, 0x7F, 0x73, 0x92, 0x7A, 0x5C, 0x6C},
};

static char saturation_ctrl2_values[NUM_SAT_CTRL][OFFSET_SAT_CTRL] = {
	{0x56, 0x66, 0x66, 0x66, 0x62, 0x60, 0x55, 0x53, 0x63, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83},
	{0x61, 0x73, 0x73, 0x73, 0x6E, 0x6C, 0x5F, 0x5E, 0x70, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83},
	{0x6C, 0x80, 0x80, 0x80, 0x7A, 0x78, 0x6A, 0x68, 0x7C, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83},
	{0x6C, 0x80, 0x80, 0x80, 0x7A, 0x78, 0x6A, 0x68, 0x7C, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83},
	{0x6C, 0x80, 0x80, 0x80, 0x7A, 0x78, 0x6A, 0x68, 0x7C, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83},
};

static char hue_ctrl1_values[NUM_HUE_CTRL][OFFSET_HUE_CTRL1] = {
	{0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x7A, 0x83, 0x83, 0x83, 0x83, 0x83, 0x7F, 0x7E, 0x7D, 0x7D, 0x7D, 0x79, 0x7F, 0x7F, 0x7E, 0x7E, 0x7D, 0x7C, 0x06, 0x11, 0x17, 0x13, 0x13, 0x27, 0xF6},
	{0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x7A, 0x83, 0x83, 0x83, 0x83, 0x83, 0x7F, 0x7F, 0x7E, 0x7E, 0x7E, 0x7B, 0x7E, 0x7F, 0x7F, 0x7F, 0x7E, 0x7D, 0xFF, 0x0A, 0x11, 0x0C, 0x0C, 0x20, 0xEF},
	{0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x7A, 0x83, 0x83, 0x83, 0x83, 0x83, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7D, 0x7C, 0x7E, 0x7E, 0x7E, 0x7F, 0x7F, 0xF4, 0x00, 0x06, 0x02, 0x01, 0x15, 0xE4},
	{0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x7A, 0x83, 0x83, 0x83, 0x83, 0x83, 0x7D, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x79, 0x7C, 0x7E, 0x7E, 0x7F, 0x7F, 0xE9, 0xF4, 0xFB, 0xF6, 0xF6, 0x0A, 0xDA},
	{0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x7A, 0x83, 0x83, 0x83, 0x83, 0x83, 0x7C, 0x7E, 0x7F, 0x7E, 0x7E, 0x7F, 0x77, 0x7B, 0x7D, 0x7D, 0x7E, 0x7F, 0xE2, 0xED, 0xF4, 0xEF, 0xEF, 0x03, 0xD3},
};

static char hue_ctrl2_values[NUM_HUE_CTRL][OFFSET_HUE_CTRL2] = {
	{0x01, 0x01, 0x01, 0x08, 0x11},
	{0xFB, 0xFB, 0xFB, 0x01, 0x0A},
	{0xEF, 0xEF, 0xEF, 0xF6, 0xFF},
	{0xE4, 0xE4, 0xE4, 0xEB, 0xF4},
	{0xDE, 0xDE, 0xDE, 0xE4, 0xED},
};

static char sha_ctrl_values[NUM_SHA_CTRL][OFFSET_SHA_CTRL] = {
	{0xFB, 0xF8, 0x30, 0x10, 0x07, 0x06, 0x04, 0x04, 0x20, 0x06, 0x10, 0x03, 0x03, 0x80, 0x04, 0x01, 0x02, 0x58, 0xE0, 0x00, 0x8F, 0x00, 0xFF, 0x7F, 0xA0, 0x00, 0xE0, 0x08, 0xC0, 0x10},
	{0xFB, 0xF8, 0x40, 0x10, 0x07, 0x06, 0x04, 0x04, 0x20, 0x06, 0x20, 0x10, 0x03, 0x80, 0x04, 0x01, 0x02, 0x58, 0xE0, 0x00, 0x8F, 0x00, 0xFF, 0x7F, 0xA0, 0x00, 0xE0, 0x08, 0xC0, 0x10},
	{0xFB, 0xF8, 0x40, 0x10, 0x07, 0x06, 0x04, 0x04, 0x20, 0x06, 0x30, 0x20, 0x03, 0x80, 0x04, 0x01, 0x02, 0x58, 0xE0, 0x00, 0x8F, 0x00, 0xFF, 0x7F, 0xA0, 0x00, 0xE0, 0x08, 0xC0, 0x10},
	{0xFB, 0xF8, 0x50, 0x20, 0x07, 0x06, 0x04, 0x04, 0x20, 0x06, 0x30, 0x20, 0x03, 0x80, 0x04, 0x01, 0x02, 0x58, 0xE0, 0x00, 0x8F, 0x00, 0xFF, 0x7F, 0xA0, 0x00, 0xE0, 0x08, 0xC0, 0x10},
	{0xFB, 0xF8, 0x60, 0x30, 0x07, 0x06, 0x04, 0x04, 0x20, 0x06, 0x30, 0x20, 0x03, 0x80, 0x04, 0x01, 0x02, 0x58, 0xE0, 0x00, 0x8F, 0x00, 0xFF, 0x7F, 0xA0, 0x00, 0xE0, 0x08, 0xC0, 0x10},
};

static void adjust_roi(struct dsi_panel *panel, int *sr, int *er)
{
	u32 cur_res = panel->cur_mode->timing.v_active;
	int type, num = panel->num_timing_nodes;

	for (type = 0; type < num; type++) {
		if (cur_res == sw43408a_res[type].height)
			break;
	}
	if (type == num) {
		pr_err("invalid height\n");
		*sr = 0;
		*er = panel->cur_mode->timing.h_active - 1;
		return;
	}

	if ((panel->lge.aod_area.w == 0) || (panel->lge.aod_area.h == 0)) {
		pr_err("area (w=%d)(h=%d), please check with application\n",
				panel->lge.aod_area.w, panel->lge.aod_area.h);
		goto full_roi;
	}

	*sr = panel->lge.aod_area.y;
	*er = *sr + panel->lge.aod_area.h - 1;

	return;

full_roi:
	*sr = 0;
	*er = *sr + sw43408a_res[0].height - 1;
	return;
}

static void prepare_cmd(struct dsi_cmd_desc *cmds, int cmds_count, int addr, int param1, int param2)
{
	struct dsi_cmd_desc *cmd = NULL;
	char *payload = NULL;

	cmd = find_cmd(cmds, cmds_count, addr);
	if (cmd) {
		payload = (char *)cmd->msg.tx_buf;
		payload++;
		WORDS_TO_BYTE_ARRAY(param1, param2, payload);
	} else {
		pr_warn("cmd for addr 0x%02X not found\n", addr);
	}
}

static void prepare_power_optimize_cmds(struct dsi_panel *panel, struct dsi_cmd_desc *cmds, int cmds_count, bool optimize)
{
	struct dsi_cmd_desc *cmd = NULL;
	char *payload = NULL;

	cmd = find_cmd(cmds, cmds_count, ADDR_WRIECTL);
	if (cmd) {
		payload = (char *)cmd->msg.tx_buf + 6;
		if (optimize)
			*payload |= BIT(2);
		else
			*payload &= ~BIT(2);
	} else {
		pr_warn("cmd for addr 0x%02X not found\n", ADDR_WRIECTL);
	}

	cmd = find_cmd(cmds, cmds_count, ADDR_PWRCTL3);
	if (cmd) {
		payload = (char *)cmd->msg.tx_buf + 5;
		if (optimize) {
			*payload &= ~BIT(5);
			payload++;
			*payload &= ~BIT(6);
			payload++;
			*payload &= ~BIT(5);
			payload++;
			*payload &= ~BIT(6);
			payload++;
			*payload &= ~BIT(5);
			payload++;
			*payload &= ~BIT(6);
			payload++;
		} else {
			*payload |= BIT(5);
			payload++;
			*payload |= BIT(6);
			payload++;
			*payload |= BIT(5);
			payload++;
			*payload |= BIT(6);
			payload++;
			*payload |= BIT(5);
			payload++;
			*payload |= BIT(6);
			payload++;
		}
	} else {
		pr_warn("cmd for addr 0x%02X not found\n", ADDR_PWRCTL3);
	}
}

static int prepare_aod_cmds_sw43408a(struct dsi_panel *panel, struct dsi_cmd_desc *cmds, int cmds_count)
{
	int rc = 0, sr = 0, er = 0;

	if (panel == NULL || cmds == NULL || cmds_count == 0)
		return -EINVAL;

	adjust_roi(panel, &sr, &er);
	prepare_cmd(cmds, cmds_count, ADDR_PTLAR, sr, er);

	if (panel->lge.aod_power_mode &&
			(panel->lge.aod_area.h != sw43408a_res[0].height)) {
		pr_info("set aod power optimization\n");
		prepare_power_optimize_cmds(panel, cmds, cmds_count, true);
	} else {
		pr_info("skip aod power optimization\n");
		prepare_power_optimize_cmds(panel, cmds, cmds_count, false);
	}

	return rc;
}

static int get_current_resolution_sw43408a(struct dsi_panel *panel)
{
	u8 reg;

	lge_mdss_dsi_panel_cmd_read(panel, (u8)ADDR_RDDISPM, 1, &reg);

	return (int)(reg & 0x03);
}

static void get_support_resolution_sw43408a(int idx, void *input)
{
	struct drs_res_info *res = (struct drs_res_info *)input;

	res->data = sw43408a_res[idx].data;
	res->height = sw43408a_res[idx].height;

	return;
}


static int control_bist_cmds_sw43408a(struct dsi_panel *panel, bool enable)
{
	int rc = 0;
	bool state_changed = false;

	if (panel == NULL)
		return -EINVAL;

	if (enable) {
		if (panel->lge.bist_on == 0) {
			state_changed = true;
		}
		panel->lge.bist_on++;
	} else {
		if (panel->lge.bist_on == 1) {
			state_changed = true;
		}
		panel->lge.bist_on--;
		if (panel->lge.bist_on < 0) {
			pr_err("FIX: bist_on has negative count = %d\n", panel->lge.bist_on);
			panel->lge.bist_on = 0;
		}
	}

	pr_info("state_changed=%d, count=%d, enable=%d\n", state_changed, panel->lge.bist_on, enable);
	if (state_changed) {
		mutex_lock(&panel->panel_lock);
		if (enable) {
			rc = lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_SET_BIST_ON);
			if (rc)
				pr_err("failed to send BIST_ON cmd, rc=%d\n", rc);
		} else {
			rc = lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_SET_BIST_OFF);
			if (rc)
				pr_err("failed to send BIST_OFF cmd, rc=%d\n", rc);
		}
		mutex_unlock(&panel->panel_lock);
	}

	return rc;
}

static int release_bist_cmds_sw43408a(struct dsi_panel *panel)
{
	int rc = 0;

	if (panel == NULL)
		return -EINVAL;

	if (panel->lge.bist_on > 0) {
		panel->lge.bist_on = 1;
		if (control_bist_cmds_sw43408a(panel, false) < 0) {
			pr_warn("fail to bist control\n");
		}
	}

	return rc;
}

static void err_detect_work_sw43408a(struct work_struct *work)
{
	struct delayed_work *dw = to_delayed_work(work);
	struct lge_dsi_panel *lge_panel = container_of(dw, struct lge_dsi_panel, err_detect_int_work);
	struct dsi_panel *panel = container_of(lge_panel, struct dsi_panel, lge);
	u8 reg;

	if (!panel) {
		pr_err("invalid ctrl data\n");
		return;
	}

	mutex_lock(&panel->panel_lock);

	if (!dsi_panel_initialized(panel)) {
		pr_err("Panel off state. Ignore screen_mode set cmd\n");
		mutex_unlock(&panel->panel_lock);
		return;
	}

	pr_info("err_detect_mask = %d\n", panel->lge.err_detect_mask);
	if (panel->lge.err_detect_mask >= 0) {
		if (panel->lge.use_ddic_reg_lock)
			lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_REGISTER_UNLOCK);
		lge_mdss_dsi_panel_cmd_read(panel, (u8)ADDR_ERR_DETECT, 1, &reg);
		if (panel->lge.use_ddic_reg_lock)
			lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_REGISTER_LOCK);

		pr_info("Reg[0x%x]=0x%x,\n", ADDR_ERR_DETECT, reg);
		panel->lge.err_detect_result = reg;
	}
	mutex_unlock(&panel->panel_lock);

	if (panel->lge.err_detect_crash_enabled && panel->lge.err_detect_result) {
		pr_err("error is detected. BUG() \n");
		BUG();
	}
}

static irqreturn_t err_detect_irq_handler_sw43408a(int irq, void *data)
{
	struct dsi_panel *panel = (struct dsi_panel *)data;

	queue_delayed_work(panel->lge.err_detect_int_workq, &panel->lge.err_detect_int_work,
	msecs_to_jiffies(50));
	return IRQ_HANDLED;
}

static int set_err_detect_mask_sw43408a(struct dsi_panel *panel)
{
	int rc = 0;
	enum lge_ddic_dsi_cmd_set_type cmd_type = LGE_DDIC_DSI_ESD_DETECT;

	if (!panel) {
		pr_err("invalid ctrl data\n");
		return -EINVAL;
	}

	if (!dsi_panel_initialized(panel)) {
		pr_err("Panel off state. Ignore screen_mode set cmd\n");
		return -EINVAL;
	}

	pr_info("mask=%d\n", panel->lge.err_detect_mask);

	switch (panel->lge.err_detect_mask) {
		case 0:
			cmd_type = LGE_DDIC_DSI_ESD_DETECT;
			break;
		case 1:
			cmd_type = LGE_DDIC_DSI_MEM_ERR_DETECT;
			break;
		case 2:
			cmd_type = LGE_DDIC_DSI_LINE_DEFECT_DETECT;
			break;
		default:
			pr_warn("out of range, set esd detect.\n");
			break;
	}

	/* UNLOCK REGISTER */
	if (panel->lge.use_ddic_reg_lock)
		lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_REGISTER_UNLOCK);

	rc = lge_ddic_dsi_panel_tx_cmd_set(panel, cmd_type);
	if (rc)
		pr_err("failed to send %d cmd, rc=%d\n", cmd_type, rc);

	/* LOCK REGISTER */
	if (panel->lge.use_ddic_reg_lock)
		lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_REGISTER_LOCK);

	return rc;
}

static void lge_set_custom_rgb_sw43408a(struct dsi_panel *panel, bool send_cmd)
{
	int i = 0;
	int red_index, green_index, blue_index = 0;
	char *payload_ctrl1 = NULL;
	char *payload_ctrl2 = NULL;

	if (panel == NULL) {
		pr_err("Invalid input\n");
		return;
	}

	mutex_lock(&panel->panel_lock);
	//cm_rgb_step : 0~11
	//rgb_index 0~11 + 0~12
	red_index   = rgb_preset[panel->lge.cm_preset_step][RED] + panel->lge.cm_red_step;
	green_index = rgb_preset[panel->lge.cm_preset_step][GREEN] + panel->lge.cm_green_step;
	blue_index  = rgb_preset[panel->lge.cm_preset_step][BLUE] + panel->lge.cm_blue_step;

	pr_info("red_index=(%d) green_index=(%d) blue_index=(%d)\n", red_index, green_index, blue_index);

	payload_ctrl1 = (char *)panel->lge.lge_cmd_sets[LGE_DDIC_DSI_DISP_DG_COMMAND_DUMMY].cmds[IDX_DG_CTRL1].msg.tx_buf;
	payload_ctrl2 = (char *)panel->lge.lge_cmd_sets[LGE_DDIC_DSI_DISP_DG_COMMAND_DUMMY].cmds[IDX_DG_CTRL2].msg.tx_buf;

	if (!payload_ctrl1 || !payload_ctrl2) {
		pr_err("LGE_DDIC_DSI_DISP_DG_COMMAND_DUMMY is NULL\n");
		mutex_unlock(&panel->panel_lock);
		return;
	}

	/*
	*	RU: RED_UPPER, BU: BLUE_UPPER, GU: GREEN_UPPER
	*	RL: RED_LOWER, BL: BLUE_LOWER, GL: GREEN_LOWER
	*
	* CTRL1(ECh): 3F RU#1~3 GU#1~3 BU#1~3 RL#1~9 GL#1~9 BL#1~4
	* CTRL2(EDh): BL#5~9
	*/

	// For RGB UPPER CTRL1
	for (i = 0; i < OFFSET_DG_UPPER; i++) {
		payload_ctrl1[i+START_DG_CTRL1] = dg_ctrl1_values[red_index][i];  //payload_ctrl1[2][3][4]
		payload_ctrl1[i+START_DG_CTRL1+OFFSET_DG_UPPER] = dg_ctrl1_values[green_index][i]; //payload_ctrl1[5][6][7]
		payload_ctrl1[i+START_DG_CTRL1+OFFSET_DG_UPPER*BLUE] = dg_ctrl1_values[blue_index][i]; //payload_ctrl1[8][9][10]
	}

	// FOR RED/GREEN LOWER CTRL1
	for (i = 0; i < OFFSET_DG_LOWER; i++) {
		payload_ctrl1[i+START_DG_CTRL1+OFFSET_DG_UPPER*RGB_ALL] = dg_ctrl2_values[red_index][i]; //payload_ctrl1[11]~[19]
		payload_ctrl1[i+START_DG_CTRL1+OFFSET_DG_UPPER*RGB_ALL+OFFSET_DG_LOWER] = dg_ctrl2_values[green_index][i]; //payload_ctrl1[20]~[28]
	}

	// For BLUE LOWER CTRL1
	for (i = 0; i < NUM_BLUE_LOWER_CTRL1; i++) {
		payload_ctrl1[i+START_DG_CTRL1+OFFSET_DG_UPPER*RGB_ALL+OFFSET_DG_LOWER*BLUE] = dg_ctrl2_values[blue_index][i]; //payload_ctrl1[29]~[32]
	}

	// For BLUE LOWER CTRL2
	for (i = 0; i < NUM_BLUE_LOWER_CTRL2; i++) {
		payload_ctrl2[i+START_DG_CTRL2] = dg_ctrl2_values[blue_index][i+NUM_BLUE_LOWER_CTRL1]; //payload_ctrl2[1]~[5]
	}

	for (i = 0; i < NUM_DG_CTRL1; i++) {
		pr_debug("Reg:0x%02x [%d:0x%02x]\n",
				REG_DG_CTRL1, i, (*(u8 *)(panel->lge.lge_cmd_sets[LGE_DDIC_DSI_DISP_DG_COMMAND_DUMMY].cmds[IDX_DG_CTRL1].msg.tx_buf+(i+1))));
	}

	for (i = 0; i < NUM_DG_CTRL2; i++) {
		pr_debug("Reg:0x%02x [%d:0x%02x]\n",
				REG_DG_CTRL2, i, (*(u8 *)(panel->lge.lge_cmd_sets[LGE_DDIC_DSI_DISP_DG_COMMAND_DUMMY].cmds[IDX_DG_CTRL2].msg.tx_buf+(i+1))));
	}

	if (send_cmd) {
		lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_DISP_DG_COMMAND_DUMMY);
	}

	mutex_unlock(&panel->panel_lock);
	return;
}

static void lge_display_control_store_sw43408a(struct dsi_panel *panel, bool send_cmd)
{
	char *payload1 = NULL;
	char *payload2 = NULL;
	char *default_payload1 = NULL;
	char *default_payload2 = NULL;

	mutex_lock(&panel->panel_lock);

	payload1 = (char *)panel->lge.lge_cmd_sets[LGE_DDIC_DSI_DISP_CTRL_COMMAND_1].cmds[0].msg.tx_buf;
	payload2 = (char *)panel->lge.lge_cmd_sets[LGE_DDIC_DSI_DISP_CTRL_COMMAND_2].cmds[0].msg.tx_buf;

	default_payload1 = (char *)panel->lge.lge_cmd_sets[LGE_DDIC_DSI_DISP_CTRL_DEFAULT_COMMAND_1].cmds[0].msg.tx_buf;
	default_payload2 = (char *)panel->lge.lge_cmd_sets[LGE_DDIC_DSI_DISP_CTRL_DEFAULT_COMMAND_2].cmds[0].msg.tx_buf;

	if (!payload1 || !payload2) {
		pr_err("LGE_DDIC_DSI_DISP_CTRL_COMMAND is NULL\n");
		mutex_unlock(&panel->panel_lock);
		return;
	}

	if (!default_payload1 || !default_payload2) {
		pr_err("LGE_DDIC_DSI_DISP_CTRL_DEFAULT_COMMAND is NULL\n");
		mutex_unlock(&panel->panel_lock);
		return;
	}

	/* CM_SEL */
	if (panel->lge.screen_mode == screen_mode_cinema)
		payload1[2] = (default_payload1[2] | screen_mode_cinema);
	else if (panel->lge.screen_mode == screen_mode_photos)
		payload1[2] = (default_payload1[2] | screen_mode_photos);
	else if (panel->lge.screen_mode == screen_mode_web)
		payload1[2] = (default_payload1[2] | screen_mode_web);
	else
		payload1[2] = (default_payload1[2] | 0x00);

	/* CE_SEL */
	if (panel->lge.use_ace_ctrl) {
		payload1[2] &= 0x0F;
		payload1[2] |= (panel->lge.ace_mode << 4);
	}

	// HDR_ON | ASE_ON
	payload2[1] = (default_payload2[1] |
					(panel->lge.ve_mode) |
					(panel->lge.color_manager_status << 1) |
					(panel->lge.sharpness_status << 3));
	// DGGMA_ON
	payload2[2] = (default_payload2[2] | (panel->lge.dgc_status << 2));

	pr_info("ctrl-command-1: 0x%02x 0x%02x",
			(*(u8 *)(panel->lge.lge_cmd_sets[LGE_DDIC_DSI_DISP_CTRL_COMMAND_1].cmds[0].msg.tx_buf+1)),
			(*(u8 *)(panel->lge.lge_cmd_sets[LGE_DDIC_DSI_DISP_CTRL_COMMAND_1].cmds[0].msg.tx_buf+2)));

	pr_info("ctrl-command-2: 0x%02x 0x%02x 0x%02x\n",
			(*(u8 *)(panel->lge.lge_cmd_sets[LGE_DDIC_DSI_DISP_CTRL_COMMAND_2].cmds[0].msg.tx_buf+1)),
			(*(u8 *)(panel->lge.lge_cmd_sets[LGE_DDIC_DSI_DISP_CTRL_COMMAND_2].cmds[0].msg.tx_buf+2)),
			(*(u8 *)(panel->lge.lge_cmd_sets[LGE_DDIC_DSI_DISP_CTRL_COMMAND_2].cmds[0].msg.tx_buf+3)));

	if (send_cmd) {
		lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_DISP_CTRL_COMMAND_1);
		lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_DISP_CTRL_COMMAND_2);
	}

	mutex_unlock(&panel->panel_lock);
	return;
}

static void lge_set_screen_tune_sw43408a(struct dsi_panel *panel)
{
	int i;
	int saturation_index = 0;
	int hue_index = 0;
	int sha_index = 0;

	char *saturation_payload1 = NULL;
	char *saturation_payload2 = NULL;

	char *hue_payload1 = NULL;
	char *hue_payload2 = NULL;

	char *sha_payload = NULL;

	mutex_lock(&panel->panel_lock);

	saturation_payload1 = (char *)panel->lge.lge_cmd_sets[LGE_DDIC_DSI_SET_SATURATION].cmds[1].msg.tx_buf;
	saturation_payload2 = (char *)panel->lge.lge_cmd_sets[LGE_DDIC_DSI_SET_SATURATION].cmds[2].msg.tx_buf;
	hue_payload1 = (char *)panel->lge.lge_cmd_sets[LGE_DDIC_DSI_SET_HUE].cmds[1].msg.tx_buf;
	hue_payload2 = (char *)panel->lge.lge_cmd_sets[LGE_DDIC_DSI_SET_HUE].cmds[2].msg.tx_buf;
	sha_payload = (char *)panel->lge.lge_cmd_sets[LGE_DDIC_DSI_SET_SHARPNESS].cmds[1].msg.tx_buf;

	if (!saturation_payload1 || !saturation_payload2) {
		pr_err("LGE_DDIC_DSI_SET_SATURATION is NULL\n");
		mutex_unlock(&panel->panel_lock);
		return;
	}

	if (!hue_payload1 || !hue_payload2) {
		pr_err("LGE_DDIC_DSI_SET_HUE is NULL\n");
		mutex_unlock(&panel->panel_lock);
		return;
	}

	if (!sha_payload) {
		pr_err("LGE_DDIC_DSI_SET_SHARPNESS is NULL\n");
		mutex_unlock(&panel->panel_lock);
		return;
	}

	saturation_index = panel->lge.sc_sat_step;
	hue_index = panel->lge.sc_hue_step;
	sha_index = panel->lge.sc_sha_step;

	// SATURATION CTRL
	for (i = 0; i < OFFSET_SAT_CTRL; i++) {
		saturation_payload1[i+1] = saturation_ctrl1_values[saturation_index][i];
		saturation_payload2[i+1] = saturation_ctrl2_values[saturation_index][i];
	}

	// HUE CTRL
	for (i = 0; i < OFFSET_HUE_CTRL1; i++) {
		hue_payload1[i+1] = hue_ctrl1_values[hue_index][i];
	}
	for (i = 0; i < OFFSET_HUE_CTRL2; i++) {
		hue_payload2[i+1] = hue_ctrl2_values[hue_index][i];
	}

	// SHARPNESS CTRL
	for (i = 0; i < OFFSET_SHA_CTRL; i++) {
		sha_payload[i+1] = sha_ctrl_values[sha_index][i];
	}


	for (i = 0; i < OFFSET_SAT_CTRL; i++) {
		pr_debug("Reg:0x%02x [%d:0x%02x]\n",
				REG_SAT_CTRL1, i, (*(u8 *)(panel->lge.lge_cmd_sets[LGE_DDIC_DSI_SET_SATURATION].cmds[1].msg.tx_buf+(i+1))));
	}
	for (i = 0; i < OFFSET_SAT_CTRL; i++) {
		pr_debug("Reg:0x%02x [%d:0x%02x]\n",
				REG_SAT_CTRL2, i, (*(u8 *)(panel->lge.lge_cmd_sets[LGE_DDIC_DSI_SET_SATURATION].cmds[2].msg.tx_buf+(i+1))));
	}
	for (i = 0; i < OFFSET_HUE_CTRL1; i++) {
		pr_debug("Reg:0x%02x [%d:0x%02x]\n",
				REG_HUE_CTRL1, i, (*(u8 *)(panel->lge.lge_cmd_sets[LGE_DDIC_DSI_SET_HUE].cmds[1].msg.tx_buf+(i+1))));
	}
	for (i = 0; i < OFFSET_HUE_CTRL2; i++) {
		pr_debug("Reg:0x%02x [%d:0x%02x]\n",
				REG_HUE_CTRL2, i, (*(u8 *)(panel->lge.lge_cmd_sets[LGE_DDIC_DSI_SET_HUE].cmds[2].msg.tx_buf+(i+1))));
	}
	for (i = 0; i < OFFSET_SHA_CTRL; i++) {
		pr_debug("Reg:0x%02x [%d:0x%02x]\n",
				REG_SHA_CTRL, i, (*(u8 *)(panel->lge.lge_cmd_sets[LGE_DDIC_DSI_SET_SHARPNESS].cmds[1].msg.tx_buf+(i+1))));
	}

	lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_SET_SATURATION);
	lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_SET_HUE);
	lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_SET_SHARPNESS);

	mutex_unlock(&panel->panel_lock);

	return;
}

static void lge_set_screen_mode_sw43408a(struct dsi_panel *panel, bool send_cmd)
{
	mutex_lock(&panel->panel_lock);

	pr_info("screen_mode %d\n", panel->lge.screen_mode);

	switch (panel->lge.screen_mode) {
	case screen_mode_auto:
		pr_info("preset: %d, red: %d, green: %d, blue: %d\n",
				panel->lge.cm_preset_step, panel->lge.cm_red_step,
				panel->lge.cm_green_step, panel->lge.cm_blue_step);

		panel->lge.sharpness_status = 0x00;
		lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_SET_SATURATION_DEFAULT);
		lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_SET_HUE_DEFAULT);

		if (panel->lge.cm_preset_step == 2 &&
			!(panel->lge.cm_red_step | panel->lge.cm_green_step | panel->lge.cm_blue_step)) {
			panel->lge.dgc_status = 0x00;
		} else {
			panel->lge.dgc_status = 0x01;
			mutex_unlock(&panel->panel_lock);
			lge_set_custom_rgb_sw43408a(panel, send_cmd);
			mutex_lock(&panel->panel_lock);
		}
		break;
	case screen_mode_sports:
		panel->lge.dgc_status = 0x01;
		panel->lge.sharpness_status = 0x00;
		lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_SET_SATURATION_DEFAULT);
		lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_SET_HUE_DEFAULT);
		lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_CM_SPORTS);
		break;
	case screen_mode_game:
		panel->lge.dgc_status = 0x01;
		panel->lge.sharpness_status = 0x00;
		lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_SET_HUE_DEFAULT);
		lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_CM_GAME);
		break;
	case screen_mode_expert:
		pr_info("saturation: %d, hue: %d, sharpness: %d\n",
				panel->lge.sc_sat_step, panel->lge.sc_hue_step, panel->lge.sc_sha_step);

		panel->lge.sharpness_status = 0x01;

		mutex_unlock(&panel->panel_lock);
		lge_set_custom_rgb_sw43408a(panel, send_cmd);
		lge_set_screen_tune_sw43408a(panel);
		mutex_lock(&panel->panel_lock);
		break;
	case screen_mode_cinema:
	case screen_mode_photos:
	case screen_mode_web:
	default:
		panel->lge.dgc_status = 0x01;
		panel->lge.sharpness_status = 0x00;
		lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_SET_SATURATION_DEFAULT);
		lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_SET_HUE_DEFAULT);
		lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_SET_WB_DEFAULT);
		break;
	}

	mutex_unlock(&panel->panel_lock);
	lge_display_control_store_sw43408a(panel, send_cmd);
}

static void sharpness_set_sw43408a(struct dsi_panel *panel, int input)
{
	mutex_lock(&panel->panel_lock);
	if (input)
		panel->lge.sharpness_status = 0x01;
	else
		panel->lge.sharpness_status = 0x00;

	lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_SET_SHARPNESS_DEFAULT);
	mutex_unlock(&panel->panel_lock);

	lge_display_control_store_sw43408a(panel, true);
}

static void lge_bc_dim_set_sw43408a(struct dsi_panel *panel, u8 bc_dim_en, u8 bc_dim_f_cnt)
{
	mutex_lock(&panel->panel_lock);
	if (bc_dim_en == BC_DIM_ON) {
		((u8 *)(panel->lge.lge_cmd_sets[LGE_DDIC_DSI_BC_DEFAULT_DIMMING].cmds[1].msg.tx_buf))[22] = bc_dim_f_cnt;
		lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_BC_DEFAULT_DIMMING);
	}
	mdelay(15);
	pr_info("FRAMES: 0x%x\n", ((u8 *)(panel->lge.lge_cmd_sets[LGE_DDIC_DSI_BC_DEFAULT_DIMMING].cmds[1].msg.tx_buf))[22]);
	mutex_unlock(&panel->panel_lock);
}

static int lge_set_therm_dim_sw43408a(struct dsi_panel *panel, int input)
{
	u8 bc_dim_en;
	u8 bc_dim_f_cnt;

	if (panel->bl_config.bd->props.brightness < BC_DIM_BRIGHTNESS_THERM) {
		pr_info("Normal Mode. Skip therm dim. Current Brightness: %d\n", panel->bl_config.bd->props.brightness);
		return -EINVAL;
	}

	bc_dim_en = BC_DIM_ON;
	if (input)
		bc_dim_f_cnt = BC_DIM_FRAMES_THERM;
	else
		bc_dim_f_cnt = BC_DIM_FRAMES_NORMAL;

	if (panel->lge.use_bc_dimming_work)
		cancel_delayed_work_sync(&panel->lge.bc_dim_work);

	lge_bc_dim_set_sw43408a(panel, bc_dim_en, bc_dim_f_cnt);

	panel->bl_config.bd->props.brightness = BC_DIM_BRIGHTNESS_THERM;
	lge_backlight_device_update_status(panel->bl_config.bd);

	if (panel->lge.use_bc_dimming_work)
		schedule_delayed_work(&panel->lge.bc_dim_work, BC_DIM_TIME);

	return 0;
}

static void lge_get_brightness_dim_sw43408a(struct dsi_panel *panel)
{
	u8 bc_dim_en;
	u8 bc_dim_f_cnt;

	bc_dim_en = true; /* always enabled */
	bc_dim_f_cnt = ((u8 *)(panel->lge.lge_cmd_sets[LGE_DDIC_DSI_BC_DEFAULT_DIMMING].cmds[1].msg.tx_buf))[22];

	pr_info("BC_DIM_EN: 0x%x, FRAMES: 0x%x\n", bc_dim_en, bc_dim_f_cnt);
}

static void lge_set_brightness_dim_sw43408a(struct dsi_panel *panel, int input)
{
	u8 bc_dim_en;
	u8 bc_dim_f_cnt;

	if (input == BC_DIM_OFF) {
		pr_warn("sw43408a do not off bc_dim feature\n");
		bc_dim_en = BC_DIM_ON;
		bc_dim_f_cnt = BC_DIM_FRAMES_NORMAL;
	} else {
		bc_dim_en = BC_DIM_ON;
		if (input < BC_DIM_MIN_FRAMES)
			bc_dim_f_cnt = BC_DIM_MIN_FRAMES;
		else if (input > BC_DIM_MAX_FRAMES)
			bc_dim_f_cnt = BC_DIM_MAX_FRAMES;
		else
			bc_dim_f_cnt = input;
	}

	lge_bc_dim_set_sw43408a(panel, bc_dim_en, bc_dim_f_cnt);
}

static void lge_set_video_enhancement_sw43408a(struct dsi_panel *panel, int input)
{
	int rc = 0;
	bool enable = false;

	panel->lge.ve_mode = input;
	enable = !!panel->lge.ve_mode;

	mutex_lock(&panel->panel_lock);
	if (enable) {
		rc = lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_VIDEO_ENHANCEMENT_ON);
		if (rc)
			pr_err("failed to send VIDEO_ENHANCEMENT_ON cmd, rc=%d\n", rc);
	}
	mutex_unlock(&panel->panel_lock);
	lge_display_control_store_sw43408a(panel, true);
	mutex_lock(&panel->panel_lock);
	if (!enable) {
		mdelay(20);
		rc = lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_VIDEO_ENHANCEMENT_OFF);
		if (rc)
			pr_err("failed to send VIDEO_ENHANCEMENT_OFF cmd, rc=%d\n", rc);
	}
	mutex_unlock(&panel->panel_lock);

	pr_info("send cmds to %s the video enhancer \n",
		(input == true) ? "enable" : "disable");

	if (panel->lge.use_bc_dimming_work)
		cancel_delayed_work_sync(&panel->lge.bc_dim_work);

	lge_bc_dim_set_sw43408a(panel, BC_DIM_ON, BC_DIM_FRAMES_VE);

	lge_backlight_device_update_status(panel->bl_config.bd);

	if (panel->lge.use_bc_dimming_work)
		schedule_delayed_work(&panel->lge.bc_dim_work, BC_DIM_TIME);
}

static int set_pps_cmds_sw43408a(struct dsi_panel *panel, enum dsi_cmd_set_type type)
{
	struct msm_display_dsc_info *dsc;

	if (!panel) {
		pr_err("panel is null\n");
		return -EINVAL;
	}

	dsc = &panel->cur_mode->priv_info->dsc;
	if (!dsc) {
		pr_err("dsc is null\n");
		return -EINVAL;
	}

	switch (type) {
	case LGE_DDIC_DSI_SET_LP1:
	case LGE_DDIC_DSI_SET_LP2:
		pr_info("LP2: pic_height : %d -> %d\n",
				dsc->pic_height,
				panel->lge.aod_area.h);
		panel->lge.pps_orig = dsc->pic_height;
		dsc->pic_height = panel->lge.aod_area.h;
		break;
	case LGE_DDIC_DSI_SET_NOLP:
		pr_info("NOLP: pic_height : %d -> %d\n",
				dsc->pic_height,
				panel->cur_mode->timing.v_active);
		panel->lge.pps_orig = dsc->pic_height;
		dsc->pic_height = panel->cur_mode->timing.v_active;
		break;
	default:
		pr_warn("WARNING: not supported type\n");
		break;
	};

	return 0;
}

static int unset_pps_cmds_sw43408a(struct dsi_panel *panel, enum dsi_cmd_set_type type)
{
	struct msm_display_dsc_info *dsc;

	if (!panel) {
		pr_err("panel is null\n");
		return -EINVAL;
	}

	dsc = &panel->cur_mode->priv_info->dsc;
	if (!dsc) {
		pr_err("dsc is null\n");
		return -EINVAL;
	}

	switch (type) {
	case LGE_DDIC_DSI_SET_LP1:
	case LGE_DDIC_DSI_SET_LP2:
	case LGE_DDIC_DSI_SET_NOLP:
		pr_info("pic_height : %d -> %d\n",
				dsc->pic_height,
				panel->lge.pps_orig);
		dsc->pic_height = panel->lge.pps_orig;
		break;
	default:
		pr_warn("WARNING: not supported type\n");
		break;
	};

	return 0;
}

void lge_check_vert_black_line_sw43408a(struct dsi_panel *panel)
{
	int rc = 0;

	mutex_lock(&panel->panel_lock);
	pr_info("send LGE_DDIC_DSI_DETECT_BLACK_VERT_LINE\n");
	rc = lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_DETECT_BLACK_VERT_LINE);
	mutex_unlock(&panel->panel_lock);

	if (rc)
		pr_err("failed to send DETECT_BLACK_VERT_LINE cmd, rc=%d\n", rc);
}

void lge_check_vert_white_line_sw43408a(struct dsi_panel *panel)
{
	int rc = 0;

	mutex_lock(&panel->panel_lock);
	pr_info("send LGE_DDIC_DSI_DETECT_WHITE_VERT_LINE\n");
	rc = lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_DETECT_WHITE_VERT_LINE);
	mutex_unlock(&panel->panel_lock);

	if (rc)
		pr_err("failed to send DETECT_WHITE_VERT_LINE cmd, rc=%d\n", rc);
}

void lge_check_vert_line_restore_sw43408a(struct dsi_panel *panel)
{
	int rc = 0;

	pr_info("%s\n", __func__);

	mutex_lock(&panel->panel_lock);
	pr_info("send LGE_DDIC_DSI_DETECT_VERT_LINE_RESTORE");
	rc = lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_DETECT_VERT_LINE_RESTORE);
	mutex_unlock(&panel->panel_lock);

	if (rc)
		pr_err("failed to send DETECT_VERT_LINE_RESTORE cmd, rc=%d\n", rc);
}

static void lge_update_irc_state(struct dsi_panel *panel, int pos, int value)
{
	int *cur_state = &(panel->lge.irc_current_state);

	if (value)
		*cur_state |= BIT(pos);
	else
		*cur_state &= ~BIT(pos);

	pr_info("success (%d), (%d)\n", *cur_state, panel->lge.irc_current_state);

	return;
}

int lge_set_irc_state_sw43408a(struct dsi_panel *panel, enum lge_irc_mode mode, enum lge_irc_ctrl enable)
{
	struct dsi_cmd_desc *cmd = NULL;
	char *payload = NULL;
	int prev_state = !!panel->lge.irc_current_state;
	int new_state;

	pr_info("irc request=%s\n", ((enable == LGE_IRC_OFF) ? "off" : "on"));

	mutex_lock(&panel->panel_lock);
	lge_update_irc_state(panel, mode, enable);
	new_state = !!panel->lge.irc_current_state;

	if (prev_state == new_state) {
		pr_info("same state, skip=(%d,%d)\n", prev_state, new_state);
		goto exit;
	}

	cmd = &(panel->lge.lge_cmd_sets[LGE_DDIC_DSI_IRC_CTRL].cmds[1]);
	if (!cmd) {
		pr_err("cmd is null\n");
		mutex_unlock(&panel->panel_lock);
		return -EINVAL;
	}
	payload = (char *)cmd->msg.tx_buf;
	payload++;

	if (enable == LGE_IRC_OFF)
		*payload &= ~BIT(5); /* Global IRC OFF */
	else
		*payload |= BIT(5); /* Global IRC ON */

	lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_IRC_CTRL);

exit:
	panel->lge.irc_pending = false;
	if ((mode == LGE_GLOBAL_IRC_HBM) && panel->lge.use_ace_ctrl) {
		if (enable == LGE_IRC_OFF) {
			panel->lge.ace_mode = 0;
			panel->lge.ve_mode = 1;
			lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_ACE_TUNE);
			mutex_unlock(&panel->panel_lock);
			lge_display_control_store_sw43408a(panel, true);
			mutex_lock(&panel->panel_lock);
		} else {
			panel->lge.ace_mode = 3;
			panel->lge.ve_mode = 0;
			mutex_unlock(&panel->panel_lock);
			lge_display_control_store_sw43408a(panel, true);
			mutex_lock(&panel->panel_lock);
			lge_ddic_dsi_panel_tx_cmd_set(panel, LGE_DDIC_DSI_ACE_RESTORE);
		}
	}
	mutex_unlock(&panel->panel_lock);
	return 0;
}

int lge_get_irc_state_sw43408a(struct dsi_panel *panel)
{
	int ret;

	mutex_lock(&panel->panel_lock);
	pr_info("current_state=%d\n", panel->lge.irc_current_state);
	ret = !!panel->lge.irc_current_state;
	mutex_unlock(&panel->panel_lock);
	return ret;
}

int lge_set_irc_default_state_sw43408a(struct dsi_panel *panel)
{
	lge_update_irc_state(panel, LGE_GLOBAL_IRC_HBM, LGE_IRC_ON);
	lge_update_irc_state(panel, LGE_GLOBAL_IRC_HDR, LGE_IRC_ON);

	return 0;
}

int hdr_mode_set_sw43408a(struct dsi_panel *panel, int input)
{
	bool hdr_mode = ((input > 0) ? true : false);

	mutex_lock(&panel->panel_lock);
	if (hdr_mode) {
		panel->lge.color_manager_status = 0;
	} else {
		panel->lge.color_manager_status = 1;
	}
	mutex_unlock(&panel->panel_lock);

	pr_info("hdr=%s, cm=%s\n", (hdr_mode ? "set" : "unset"),
			((panel->lge.color_manager_status == 1) ? "enabled" : "disabled"));

	lge_display_control_store_sw43408a(panel, true);
	if (panel->lge.use_irc_ctrl) {
		if (hdr_mode) {
			lge_set_irc_state_sw43408a(panel, LGE_GLOBAL_IRC_HDR, LGE_IRC_OFF);
		} else {
			lge_set_irc_state_sw43408a(panel, LGE_GLOBAL_IRC_HDR, LGE_IRC_ON);
		}
	}

	return 0;
}

struct lge_ddic_ops sw43408a_ops = {
	.store_aod_area = store_aod_area,
	.prepare_aod_cmds = prepare_aod_cmds_sw43408a,
	.hdr_mode_set = hdr_mode_set_sw43408a,
	.bist_ctrl = control_bist_cmds_sw43408a,
	.release_bist = release_bist_cmds_sw43408a,
	.get_current_res = get_current_resolution_sw43408a,
	.get_support_res = get_support_resolution_sw43408a,
	.err_detect_work = err_detect_work_sw43408a,
	.err_detect_irq_handler = err_detect_irq_handler_sw43408a,
	.set_err_detect_mask = set_err_detect_mask_sw43408a,
	.lge_set_custom_rgb = lge_set_custom_rgb_sw43408a,
	.lge_display_control_store = lge_display_control_store_sw43408a,
	.lge_set_screen_tune = lge_set_screen_tune_sw43408a,
	.lge_set_screen_mode = lge_set_screen_mode_sw43408a,
	.sharpness_set = sharpness_set_sw43408a,
	.lge_bc_dim_set = lge_bc_dim_set_sw43408a,
	.lge_set_therm_dim = lge_set_therm_dim_sw43408a,
	.lge_get_brightness_dim = lge_get_brightness_dim_sw43408a,
	.lge_set_brightness_dim = lge_set_brightness_dim_sw43408a,
	.lge_set_video_enhancement = lge_set_video_enhancement_sw43408a,
	.set_pps_cmds = set_pps_cmds_sw43408a,
	.unset_pps_cmds = unset_pps_cmds_sw43408a,
	.lge_check_vert_black_line = lge_check_vert_black_line_sw43408a,
	.lge_check_vert_white_line = lge_check_vert_white_line_sw43408a,
	.lge_check_vert_line_restore = lge_check_vert_line_restore_sw43408a,
	.set_irc_default_state = lge_set_irc_default_state_sw43408a,
	.set_irc_state = lge_set_irc_state_sw43408a,
	.get_irc_state = lge_get_irc_state_sw43408a,
};
