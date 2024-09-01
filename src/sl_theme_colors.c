
struct sl_css_color {
    const char *name;
    const char *color;
};

#define DEFINE_CSS_COLOR(v, n)   \
    { .name = n, .color = v}

struct sl_css_color sl_css_named_colors[] = {
    DEFINE_CSS_COLOR("#ffdead", "navajowhite"),
    DEFINE_CSS_COLOR("#000080", "navy"),
    DEFINE_CSS_COLOR("#fdf5e6", "oldlace"),
    DEFINE_CSS_COLOR("#808000", "olive"),
    DEFINE_CSS_COLOR("#6b8e23", "olivedrab"),
    DEFINE_CSS_COLOR("#ffa500", "orange"),
    DEFINE_CSS_COLOR("#ff4500", "orangered"),
    DEFINE_CSS_COLOR("#da70d6", "orchid"),
    DEFINE_CSS_COLOR("#eee8aa", "palegoldenrod"),
    DEFINE_CSS_COLOR("#98fb98", "palegreen"),
    DEFINE_CSS_COLOR("#afeeee", "paleturquoise"),
    DEFINE_CSS_COLOR("#db7093", "palevioletred"),
    DEFINE_CSS_COLOR("#ffefd5", "papayawhip"),
    DEFINE_CSS_COLOR("#ffdab9", "peachpuff"),
    DEFINE_CSS_COLOR("#cd853f", "peru"),
    DEFINE_CSS_COLOR("#ffc0cb", "pink"),
    DEFINE_CSS_COLOR("#dda0dd", "plum"),
    DEFINE_CSS_COLOR("#b0e0e6", "powderblue"),
    DEFINE_CSS_COLOR("#800080", "purple"),
    DEFINE_CSS_COLOR("#663399", "rebeccapurple"),
    DEFINE_CSS_COLOR("#ff0000", "red"),
    DEFINE_CSS_COLOR("#bc8f8f", "rosybrown"),
    DEFINE_CSS_COLOR("#4169e1", "royalblue"),
    DEFINE_CSS_COLOR("#8b4513", "saddlebrown"),
    DEFINE_CSS_COLOR("#fa8072", "salmon"),
    DEFINE_CSS_COLOR("#f4a460", "sandybrown"),
    DEFINE_CSS_COLOR("#2e8b57", "seagreen"),
    DEFINE_CSS_COLOR("#fff5ee", "seashell"),
    DEFINE_CSS_COLOR("#a0522d", "sienna"),
    DEFINE_CSS_COLOR("#c0c0c0", "silver"),
    DEFINE_CSS_COLOR("#87ceeb", "skyblue"),
    DEFINE_CSS_COLOR("#6a5acd", "slateblue"),
    DEFINE_CSS_COLOR("#708090", "slategray"),
    DEFINE_CSS_COLOR("#708090", "slategrey"),
    DEFINE_CSS_COLOR("#fffafa", "snow"),
    DEFINE_CSS_COLOR("#00ff7f", "springgreen"),
    DEFINE_CSS_COLOR("#4682b4", "steelblue"),
    DEFINE_CSS_COLOR("#d2b48c", "tan"),
    DEFINE_CSS_COLOR("#008080", "teal"),
    DEFINE_CSS_COLOR("#d8bfd8", "thistle"),
    DEFINE_CSS_COLOR("#ff6347", "tomato"),
    DEFINE_CSS_COLOR("#40e0d0", "turquoise"),
    DEFINE_CSS_COLOR("#ee82ee", "violet"),
    DEFINE_CSS_COLOR("#f5deb3", "wheat"),
    DEFINE_CSS_COLOR("#ffffff", "white"),
    DEFINE_CSS_COLOR("#f5f5f5", "whitesmoke"),
    DEFINE_CSS_COLOR("#ffff00", "yellow"),
    DEFINE_CSS_COLOR("#9acd32", "yellowgreen"),
};