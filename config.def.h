/* Taken from https://github.com/djpohly/dwl/issues/466 */
#include <libinput.h>
#define COLOR(hex)    { ((hex >> 24) & 0xFF) / 255.0f, \
                        ((hex >> 16) & 0xFF) / 255.0f, \
                        ((hex >> 8) & 0xFF) / 255.0f, \
                        (hex & 0xFF) / 255.0f }
/* appearance */
static const int sloppyfocus               = 0;		/* focus follows mouse */
static const int bypass_surface_visibility = 0;		/* 1 means idle inhibitors will disable idle tracking even if it's surface isn't visible  */
static const unsigned int borderpx         = 2;		/* border pixel of windows */
static const unsigned int roundedpx		   = 4;		/* rounded corners on windows */
static const unsigned int msep			   = 4;	/* seperation between window borders with edge of monitor */
static const unsigned int wsep			   = 3;		/* seperation between window borders with eachother and edge of monitor */
static const float rootcolor[]             = COLOR(0x00000000);
static const float bordercolor[]           = COLOR(0x56526e99); /* hypr: 0x595959aa pine: 0x31748f50 */
static const float focuscolor[]            = COLOR(0x908caa77); /* rose: 0xebbcbaaa foam: 0x9ccfd8aa */
static const float urgentcolor[]           = COLOR(0xeb6f9299); /* love: 0xeb6f92dd */
/* To conform the xdg-protocol, set the alpha to zero to restore the old behavior */
static const float fullscreen_bg[]         = {0.1f, 0.1f, 0.1f, 1.0f}; /* You can also use glsl colors */
static char *const wbg_argv[]			   = { "/usr/local/bin/wbg", "/home/basil/images/jelly-fish-space-fix.png", NULL };
static char *const waybar_argv[]		   = { "waybar", NULL };
static char *const firefox_argv[]		   = { "firefox", NULL };
static char *const discord_argv[]		   = { "discord", "--enable-features=UseOzonePlatform", "--ozone-platform=wayland", NULL };
static const unsigned int mouse_sens = 16;

/* scenefx variables */
static const int opacity = 0; /* flag to enable opacity */
static const float opacity_inactive = 0.5;
static const float opacity_active = 1.0;

static const int shadow = 1; /* flag to enable shadow */
static const int shadow_only_floating = 0; /* only apply shadow to floating windows */
static const struct wlr_render_color shadow_color = COLOR(0x0000FFff);
static const struct wlr_render_color shadow_color_focus = COLOR(0xFF0000ff);
static const int shadow_blur_sigma = 20;
static const int shadow_blur_sigma_focus = 40;
static const char *const shadow_ignore_list[] = { "xdg-desktop-portal-gtk", NULL }; /* list of app-id to ignore */

static const int corner_radius = 0; /* 0 disables corner_radius */

static const int blur = 1; /* flag to enable blur */
static const int blur_optimized = 1;
static const int blur_ignore_transparent = 1;
static const struct blur_data blur_data = {
	.radius = 5,
	.num_passes = 3,
	.noise = (float)0.02,
	.brightness = (float)0.9,
	.contrast = (float)0.9,
	.saturation = (float)1.1,
};

/* tagging - TAGCOUNT must be no greater than 31 */
#define TAGCOUNT (10)

/* logging */
static int log_level = WLR_ERROR;

static const Rule rules[] = {
	/* app_id     title       tags mask     isfloating   monitor */
	/* examples:
	{ "Gimp",     NULL,       0,            1,           -1 },
	*/
	{ "firefox", NULL, 0, 0, -1 }
};

/* layout(s) */
static const Layout layouts[] = {
	/* symbol, arrange function */
	{ "", tile },
	{ "[]+", split }
};

/* monitors */
static const MonitorRule monrules[] = {
    { "HDMI-A-1", 0.5f,  1,      1,    &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL,   -1,  -1 },
    { "DP-1",     0.5f,  1,      1,    &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL,   -1,  -1 },
	/* defaults */
	{ NULL,       0.5f,  1,      1,    &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL,   -1,  -1 },
};

/* keyboard */
static const struct xkb_rule_names xkb_rules = {
	.options = NULL,
    .layout = "us",
};

static const int repeat_rate = 25;
static const int repeat_delay = 400;

/* Trackpad */
static const int tap_to_click = 1;
static const int tap_and_drag = 1;
static const int drag_lock = 1;
static const int natural_scrolling = 0;
static const int disable_while_typing = 1;
static const int left_handed = 0;
static const int middle_button_emulation = 0;
/* You can choose between:
LIBINPUT_CONFIG_SCROLL_NO_SCROLL
LIBINPUT_CONFIG_SCROLL_2FG
LIBINPUT_CONFIG_SCROLL_EDGE
LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN
*/
static const enum libinput_config_scroll_method scroll_method = LIBINPUT_CONFIG_SCROLL_2FG;

/* You can choose between:
LIBINPUT_CONFIG_CLICK_METHOD_NONE
LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS
LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER
*/
static const enum libinput_config_click_method click_method = LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS;

/* You can choose between:
LIBINPUT_CONFIG_SEND_EVENTS_ENABLED
LIBINPUT_CONFIG_SEND_EVENTS_DISABLED
LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE
*/
static const uint32_t send_events_mode = LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;

/* You can choose between:
LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT
LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE
*/
static const enum libinput_config_accel_profile accel_profile = LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT;
static const double accel_speed = 0.0;
/* You can choose between:
LIBINPUT_CONFIG_TAP_MAP_LRM -- 1/2/3 finger tap maps to left/right/middle
LIBINPUT_CONFIG_TAP_MAP_LMR -- 1/2/3 finger tap maps to left/middle/right
*/
static const enum libinput_config_tap_button_map button_map = LIBINPUT_CONFIG_TAP_MAP_LRM;

/* If you want to use the windows key for MODKEY, use WLR_MODIFIER_LOGO */
#define MODKEY WLR_MODIFIER_LOGO

#define TAGKEYS(KEY,SKEY,TAG) \
	{ MODKEY,                    KEY,            view,            {.ui = 1 << TAG} }, \
	{ MODKEY,                    SKEY,           view,            {.ui = 1 << TAG} }, \
	/*{ MODKEY|WLR_MODIFIER_CTRL,  KEY,            toggleview,      {.ui = 1 << TAG} },*/ \
	{ MODKEY|WLR_MODIFIER_CTRL,  KEY,            tag,             {.ui = 1 << TAG} }, \
	{ MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT,SKEY,toggletag, {.ui = 1 << TAG} }

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static const char *termcmd[] = { "alacritty", NULL };
static const char *menucmd[] = { "wofi", NULL };
static const char *grimcmd[] = { "/usr/local/bin/grimctl", NULL };
static const char *decrease_volume[] = { "/usr/bin/pactl", "set-sink-volume", "0", "-5%", NULL };
static const char *increase_volume[] = { "/usr/bin/pactl", "set-sink-volume", "0", "+5%", NULL };
static const char *toggle_waybar[] = { "killall", "-SIGUSR1", "waybar" };

static const Key keys[] = {
	/* Note that Shift changes certain key codes: c -> C, 2 -> at, etc. */
	/* modifier                  key                 function		   argument */
	{ MODKEY,                    XKB_KEY_n,          spawn,			   {.v = menucmd} },
	{ MODKEY,                    XKB_KEY_Return,     spawn,            {.v = termcmd} },
	{ MODKEY,                    XKB_KEY_y,			 spawn,            {.v = grimcmd} },
	{ MODKEY,                    XKB_KEY_minus,		 spawn,            {.v = decrease_volume} },
	{ MODKEY,                    XKB_KEY_equal,		 spawn,            {.v = increase_volume} },
	{ MODKEY,                    XKB_KEY_space,		 spawn,            {.v = toggle_waybar} },
	{ MODKEY,                    XKB_KEY_j,          focusstack,       {.i =  1} },
	{ MODKEY,                    XKB_KEY_k,          focusstack,       {.i = -1} },
	{ MODKEY,                    XKB_KEY_bracketleft,          incnmaster,       {.i =  1} },
	{ MODKEY,                    XKB_KEY_bracketright,          incnmaster,       {.i = -1} },
	{ MODKEY,                    XKB_KEY_h,          setmfact,         {.f = -0.05f} },
	{ MODKEY,                    XKB_KEY_l,          setmfact,         {.f =  0.05f} },
	{ MODKEY,					 XKB_KEY_m,			 zoom,             {0} },
	{ MODKEY,                    XKB_KEY_Tab,        view,             {.ui = ~0} },
	{ MODKEY,                    XKB_KEY_c,          killclient,       {0} },
	//{ MODKEY,                    XKB_KEY_b,          togglebar,        {0}},
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_T,          setlayout,        {.v = &layouts[0]} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_G,          setlayout,        {.v = &layouts[1]} },
	{ MODKEY,                    XKB_KEY_space,      setlayout,		   {0} },
	{ MODKEY,                    XKB_KEY_f,          togglefullscreen, {0} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_parenright, tag,              {.ui = ~0} },
	{ MODKEY,                    XKB_KEY_semicolon,  focusmon,         {.i = WLR_DIRECTION_RIGHT} },
	{ MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_semicolon,	 tagmon,		   {.i = WLR_DIRECTION_RIGHT} },
	TAGKEYS(XKB_KEY_u, XKB_KEY_U,			0),
	TAGKEYS(XKB_KEY_i, XKB_KEY_I,			1),
	TAGKEYS(XKB_KEY_o, XKB_KEY_O,			2),
	TAGKEYS(XKB_KEY_p, XKB_KEY_P,			3),
	TAGKEYS(XKB_KEY_5, XKB_KEY_asciicircum,	4),
	TAGKEYS(XKB_KEY_6, XKB_KEY_asciicircum,	5),
	TAGKEYS(XKB_KEY_7, XKB_KEY_ampersand,	6),
	TAGKEYS(XKB_KEY_8, XKB_KEY_asterisk,	7),
	TAGKEYS(XKB_KEY_9, XKB_KEY_parenleft,	8),
	TAGKEYS(XKB_KEY_0, XKB_KEY_parenright,	9),
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Q,          quit,           {0} },

	/* Ctrl-Alt-Backspace and Ctrl-Alt-Fx used to be handled by X server */
	{ WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_Terminate_Server, quit, {0} },
	/* Ctrl-Alt-Fx is used to switch to another VT, if you don't know what a VT is
	 * do not remove them.
	 */
#define CHVT(n) { WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_XF86Switch_VT_##n, chvt, {.ui = (n)} }
	CHVT(1), CHVT(2), CHVT(3), CHVT(4), CHVT(5), CHVT(6),
	CHVT(7), CHVT(8), CHVT(9), CHVT(10), CHVT(11), CHVT(12),
};

static const Button buttons[] = {
	{ MODKEY, BTN_LEFT,   moveresize,     {.ui = CurMove} },
	{ MODKEY, BTN_MIDDLE, togglefloating, {0} },
	{ MODKEY, BTN_RIGHT,  moveresize,     {.ui = CurResize} },
};
