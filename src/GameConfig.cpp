#include "GameConfig.hpp"
#include <IconsLucide.h>
#include <map>

const GameConfig Platinum {
    "EUPC",
    0xE601C,
    "fielddata/areadata/area_data.narc",
    "fielddata/maptable/mapname.bin",
    "fielddata/land_data/land_data.narc",
    "fielddata/areadata/area_map_tex/map_tex_set.narc",
    "fielddata/build_model/build_model.narc",
    "fielddata/mapmatrix/map_matrix.narc",
    "fielddata/eventdata/zone_event.narc",
    "fielddata/encountdata/pl_enc_data.narc",
    "msgdata/pl_msg.narc",
    "data/mmodel/mmodel.narc",
    "fielddata/mm_list/move_model_list.narc",
    "(none)",
    "arc/area_win_gra.narc",
    "poketool/icongra/pl_poke_icon.narc",
    433,
    412,
    false,
    0xF0780
};

const GameConfig SoulSilver {
    "EGPI",
    0xF6BE0,
    "a/0/4/2", //area data
    "fielddata/maptable/mapname.bin", //???
    "a/0/6/5", //landdata
    "a/0/4/4", // tex set
    "a/0/4/0", // build models
    "a/0/4/1", //matrix
    "a/0/3/2", // zone event
    "a/1/3/6", //encounters
    "a/0/2/7", // message
    "a/0/8/1", //move model
    "(none)",
    "a/1/4/8",
    "arc/area_win_gra.narc",
    "poketool/icongra/pl_poke_icon.narc",
    279,
    237,
    true,
    0xF0780
};


std::map<uint32_t, const GameConfig> Configs {
    { 0x45555043, Platinum },
    { 0x45475049, SoulSilver },
//    { (uint32_t)'IRBO', Black }
};

uint32_t mCurrentGameConfig { 0x45555043 };

void CurrentGameConfig(uint32_t config){
    mCurrentGameConfig = config;
}

uint32_t CurrentGameConfig(){
    return mCurrentGameConfig;
}

///
/// Platinum
/// 
std::map<uint8_t, std::string> PlatWeatherLabels = {
    { 0x00, ICON_LC_CLOUD_OFF " Normal" },
    { 0x01, ICON_LC_CLOUD_SUN " Partly Cloudy" },
    { 0x02, ICON_LC_CLOUD_RAIN " Rain" },
    { 0x03, ICON_LC_CLOUD_RAIN_WIND " Downpour" },
    { 0x04, ICON_LC_CLOUD_LIGHTNING " Thunderstorm" },
    { 0x05, ICON_LC_CLOUD_SNOW " Light Snow" },
    { 0x06, ICON_LC_SPARKLES " Diamond Dust" },
    { 0x07, ICON_LC_SNOWFLAKE " Blizzard" },
    { 0x08, ICON_LC_UMBRELLA " Weather 8" },
    { 0x09, ICON_LC_MOUNTAIN " Volcanic Ash" },
    { 0x0A, ICON_LC_TORNADO " Sandstorm" },
    { 0x0B, ICON_LC_CLOUD_HAIL " Hail" },
    { 0x0C, ICON_LC_STONE " Rocks" },
    { 0x0D, ICON_LC_UMBRELLA " Weather 13" },
    { 0x0E, ICON_LC_CLOUD_FOG " Fog" },
    { 0x0F, ICON_LC_CLOUD_FOG " Deep Fog" },
    { 0x10, ICON_LC_FLASHLIGHT " Dark (Flashable)" },
    { 0x11, ICON_LC_ZAP " Lightning" },
    { 0x12, ICON_LC_CLOUD_FOG " Light Fog" },
    { 0x13, ICON_LC_CLOUD_FOG " Heavy Fog" },
    { 0x14, ICON_LC_UMBRELLA " Weather 20" },
    { 0x15, ICON_LC_SPARKLES " Diamond Dust 2" },
    { 0x16, ICON_LC_MOUNTAIN " Heavy Volcanic Ash" },
    { 0x17, ICON_LC_CLOUDY " Very Cloudy" },
    { 0x18, ICON_LC_SPOTLIGHT " Spotlight 1" },
    { 0x19, ICON_LC_SPOTLIGHT " Spotlight 2" },
    { 0x1A, ICON_LC_CLOUD_FOG " Dark Fog" },
    { 0x1B, ICON_LC_PAINT_BUCKET " Green Overlay" },
    { 0x1C, ICON_LC_PAINT_BUCKET " Red Overlay" },
    { 0x1D, ICON_LC_PAINT_BUCKET " Blue Overlay" },
    { 0x1E, ICON_LC_MOON " Dim" },
    { 0x1F, ICON_LC_UMBRELLA " Weather 31" },
    { 0x20, ICON_LC_CLOUD_RAIN " Rain 2" },
    { 0x21, ICON_LC_UMBRELLA " Weather 33" },
    { 0x22, ICON_LC_SPARKLES " Diamond Dust 3" },
    { 0x23, ICON_LC_SPARKLES " Diamond Dust 4" },
    { 0x24, ICON_LC_CLOUD_SNOW " Light Snow 2" },
};

std::map<uint16_t, std::string> DPPlatBGM = {
    { 1000, "Mystery Zone" },
    { 1001, "Silence" },
    { 1002, "Silence (Dungeon)" },
    { 1004, "Twinleaf Town (Day)" },
    { 1005, "Sandgem Town (Day)" },
    { 1006, "Floaroma Town (Day)" },
    { 1007, "Solaceon Town (Day)" },
    { 1008, "Route 225 (Day)" },
    { 1009, "Valor Lakefront (Day)" },
    { 1010, "Jubilife City (Day)" },
    { 1011, "Canalave City (Day)" },
    { 1012, "Oreburgh City (Day)" },
    { 1013, "Eterna City (Day)" },
    { 1014, "Hearthome City (Day)" },
    { 1015, "Pastoria City (Day)" },
    { 1016, "Veilstone City (Day)" },
    { 1017, "Sunyshore City (Day)" },
    { 1018, "Snowpoint City (Day)" },
    { 1019, "Pokémon League (Day)" },
    { 1020, "Fight Area (Day)" },
    { 1021, "Route 201 (Day)" },
    { 1022, "Route 203 (Day)" },
    { 1023, "Route 205 (Day)" },
    { 1024, "Route 206 (Day)" },
    { 1025, "Route 209 (Day)" },
    { 1026, "Route 215 (Day)" },
    { 1027, "Route 216 (Day)" },
    { 1028, "Route 228 (Day)" },
    { 1029, "Rowan Opening Theme" },
    { 1030, "Twinleaf Town (Night)" },
    { 1031, "Eterna City (Day - Variation)" },
    { 1032, "Route 205 (Day - Variation)" },
    { 1033, "Twinleaf Town (Night)" },
    { 1034, "Sandgem Town (Night)" },
    { 1035, "Floaroma Town (Night)" },
    { 1036, "Pastoria City (Night)" },
    { 1037, "Route 225 (Night)" },
    { 1038, "Valor Lakefront (Night)" },
    { 1039, "Jubilife City (Night)" },
    { 1040, "Canalave City (Night)" },
    { 1041, "Oreburgh City (Night)" },
    { 1042, "Eterna City (Night)" },
    { 1043, "Hearthome CIty (Night)" },
    { 1044, "Pastoria City (Night)" },
    { 1045, "Veilstone City (Night)" },
    { 1046, "Sunyshore City (Night)" },
    { 1047, "Snowpoint City (Night)" },
    { 1048, "Pokémon League (Night)" },
    { 1049, "Fight Area (Night)" },
    { 1050, "Route 201 (Night)" },
    { 1051, "Route 203 (Night)" },
    { 1052, "Route 205 (Night)" },
    { 1053, "Route 206 (Night)" },
    { 1054, "Route 209 (Night)" },
    { 1055, "Route 215 (Night)" },
    { 1056, "Route 216 (Night)" },
    { 1057, "Route 228 (Night)" },
    { 1058, "Eterna City (Night - Variation)" },
    { 1059, "Route 205 (Night - Variation)" },
    { 1060, "The Underground" },
    { 1061, "Captured a Flag!" },
    { 1062, "Victory Road" },
    { 1063, "Eterna Forest" },
    { 1064, "Old Chateau" },
    { 1065, "Cavern on the Lake" },
    { 1066, "Amity Square" },
    { 1067, "Team Galactic HQ" },
    { 1068, "Galactic Eterna Building" },
    { 1069, "Great Marsh" },
    { 1070, "Lake theme (Day)" },
    { 1071, "Mt. Coronet" },
    { 1072, "Spear Pillar" },
    { 1073, "Stark Mountain (in)" },
    { 1074, "Cave 1" },
    { 1075, "Cave 2" },
    { 1076, "Elite 4 - Showdown" },
    { 1077, "Hall of Fame" },
    { 1078, "Victory Road [Unused 1]" },
    { 1080, "Victory Road [Unused 2]" },
    { 1081, "Pokémon Center (Day)" },
    { 1073, "Stark Mountain (in) [Unused]" },
    { 1084, "Hall of Fame [Unused]" },
    { 1085, "Pokémon Center (Day)" },
    { 1086, "Pokémon Center (Night)" },
    { 1087, "Gym theme" },
    { 1088, "Rowan's Lab" },
    { 1089, "Poffin House" },
    { 1090, "Pokémon Mart" },
    { 1091, "Game Corner" },
    { 1092, "Battle Tower (inside)" },
    { 1093, "Jubilife TV" },
    { 1094, "Team Galactic Lab" },
    { 1096, "Hall of Origin" },
    { 1097, "GTS theme" },
    { 1190, "Distortion World" },
    { 1191, "Battle Arcade" },
    { 1192, "Battle Hall" },
    { 1193, "Battle Castle" },
    { 1194, "Battle Castle [Unused]" },
    { 1195, "Battle Factory" },
    { 1196, "Global Terminal" },
    { 1198, "Lilycove City (Bossa Nova)" },
    { 1212, "Wi-Fi Plaza" },
    { 1213, "Wi-Fi Plaza: Parade" },
    { 1218, "Twinleaf Town (Music Box) "}
};