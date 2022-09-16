#include "common\\common.h"
#include "provinces\\provinces_io.hpp"
#include "Parsers\\parsers.hpp"
#include "object_parsing\\object_parsing.hpp"
#include "modifiers\\modifiers_io.h"
#include <SOIL/SOIL.h>
#include <Windows.h>
#undef max
#undef min
#include "scenario\\scenario.h"
#include "province_functions.h"
#include "nations\\nations_functions.h"
#include "world_state\\world_state.h"
#include "graphics\\world_map.h"


void serialization::serializer<provinces::provinces_state>::serialize_object(std::byte *& output, provinces::provinces_state const & obj, world_state const & ws) {
	serialize(output, obj.province_state_container, ws);
	serialize(output, obj.party_loyalty);
	serialize(output, obj.is_canal_enabled);
}

void serialization::serializer<provinces::provinces_state>::deserialize_object(std::byte const *& input, provinces::provinces_state & obj, world_state & ws) {
	deserialize(input, obj.province_state_container, ws);
	deserialize(input, obj.party_loyalty);
	deserialize(input, obj.is_canal_enabled);

	{
		auto name = text_data::get_thread_safe_existing_text_handle(ws.s.gui_m.text_data_sequences, "europe");
		if(auto f = ws.s.modifiers_m.named_provincial_modifiers_index.find(name); f != ws.s.modifiers_m.named_provincial_modifiers_index.end())
			obj.europe_modifier = f->second;
	}
	{
		auto name = text_data::get_thread_safe_existing_text_handle(ws.s.gui_m.text_data_sequences, "asia");
		if(auto f = ws.s.modifiers_m.named_provincial_modifiers_index.find(name); f != ws.s.modifiers_m.named_provincial_modifiers_index.end())
			obj.asia_modifier = f->second;
	}
	{
		auto name = text_data::get_thread_safe_existing_text_handle(ws.s.gui_m.text_data_sequences, "north_america");
		if(auto f = ws.s.modifiers_m.named_provincial_modifiers_index.find(name); f != ws.s.modifiers_m.named_provincial_modifiers_index.end())
			obj.north_america_modifier = f->second;
	}
	{
		auto name = text_data::get_thread_safe_existing_text_handle(ws.s.gui_m.text_data_sequences, "south_america");
		if(auto f = ws.s.modifiers_m.named_provincial_modifiers_index.find(name); f != ws.s.modifiers_m.named_provincial_modifiers_index.end())
			obj.south_america_modifier = f->second;
	}
	{
		auto name = text_data::get_thread_safe_existing_text_handle(ws.s.gui_m.text_data_sequences, "africa");
		if(auto f = ws.s.modifiers_m.named_provincial_modifiers_index.find(name); f != ws.s.modifiers_m.named_provincial_modifiers_index.end())
			obj.africa_modifier = f->second;
	}
	{
		auto name = text_data::get_thread_safe_existing_text_handle(ws.s.gui_m.text_data_sequences, "oceania");
		if(auto f = ws.s.modifiers_m.named_provincial_modifiers_index.find(name); f != ws.s.modifiers_m.named_provincial_modifiers_index.end())
			obj.oceania_modifier = f->second;
	}
}

size_t serialization::serializer<provinces::provinces_state>::size(provinces::provinces_state const & obj, world_state const & ws) {
	return serialize_size(obj.province_state_container, ws) +
		serialize_size(obj.party_loyalty) +
		serialize_size(obj.is_canal_enabled);
}

namespace provinces {
	using factory_level_pair = std::pair<economy::factory_type_tag, int32_t>;
	using ideology_loyalty_pair = std::pair<ideologies::ideology_tag, float>;

	struct province_history_block;

	void merge_province_history_blocks(province_history_block& base_block, province_history_block const& new_block);

	struct province_history_environment {
		scenario::scenario_manager& s;
		date_tag target_date;

		province_history_environment(scenario::scenario_manager& sm, date_tag d) : s(sm), target_date(d) {}
	};

	struct factory_pair_reader {
		province_history_environment& env;
		int32_t level = 0;
		economy::factory_type_tag building;

		factory_pair_reader(province_history_environment& e) : env(e) {}

		void set_building(const token_and_type& t) {
			building = tag_from_text(
				env.s.economy_m.named_factory_types_index,
				text_data::get_thread_safe_existing_text_handle(env.s.gui_m.text_data_sequences, t.start, t.end));
		}
		void discard(int) {}
	};

	struct party_loyalty_reader {
		province_history_environment& env;
		float loyalty_value = 0.0f;
		ideologies::ideology_tag ideology;

		party_loyalty_reader(province_history_environment& e) : env(e) {}

		void set_ideology(const token_and_type& t) {
			ideology = tag_from_text(
				env.s.ideologies_m.named_ideology_index,
				text_data::get_thread_safe_existing_text_handle(env.s.gui_m.text_data_sequences, t.start, t.end));
		}
	};

	struct province_history_block {
		province_history_environment& env;
		province_history_block(province_history_environment& e) : env(e) {}

		economy::goods_tag trade_goods;
		std::optional<int16_t> life_rating = std::optional<int16_t>();
		cultures::national_tag owner;
		cultures::national_tag controller;
		modifiers::provincial_modifier_tag terrain;
		std::vector<cultures::national_tag> add_cores;
		std::vector<cultures::national_tag> remove_cores;
		std::vector<factory_level_pair> factories;
		std::optional<int32_t> fort_level = std::optional<int32_t>();
		std::optional<int32_t> naval_base_level = std::optional<int32_t>();
		std::optional<int32_t> railroad_level = std::optional<int32_t>();
		std::vector<ideology_loyalty_pair> party_loyalty;
		std::optional<int32_t> colony = std::optional<int32_t>();

		void discard(int) {}
		void add_dated_block(std::pair<token_and_type, province_history_block> const& p) {
			const auto date_tag = parse_date(p.first.start, p.first.end);
			if(date_tag <= env.target_date)
				merge_province_history_blocks(*this, p.second);
		}
		void set_trade_goods(const token_and_type& t) {
			trade_goods = tag_from_text(
				env.s.economy_m.named_goods_index,
				text_data::get_thread_safe_existing_text_handle(env.s.gui_m.text_data_sequences, t.start, t.end));
		}
		void set_owner(const token_and_type& t) {
			owner = tag_from_text(
				env.s.culture_m.national_tags_index,
				cultures::tag_to_encoding(t.start, t.end));
		}
		void set_controller(const token_and_type& t) {
			controller = tag_from_text(
				env.s.culture_m.national_tags_index,
				cultures::tag_to_encoding(t.start, t.end));
		}
		void set_terrain(const token_and_type& t) {
			terrain = tag_from_text(
				env.s.modifiers_m.named_provincial_modifiers_index,
				text_data::get_thread_safe_existing_text_handle(env.s.gui_m.text_data_sequences, t.start, t.end));
		}
		void add_core(const token_and_type& t) {
			add_cores.push_back(tag_from_text(
				env.s.culture_m.national_tags_index,
				cultures::tag_to_encoding(t.start, t.end)));
		}
		void remove_core(const token_and_type& t) {
			const auto tag = tag_from_text(
				env.s.culture_m.national_tags_index,
				cultures::tag_to_encoding(t.start, t.end));

			if(auto f = std::find(add_cores.begin(), add_cores.end(), tag); f != add_cores.end()) {
				*f = add_cores.back();
				add_cores.pop_back();
			} else {
				remove_cores.push_back(tag);
			}
		}
		void add_factory_pair(factory_pair_reader const& fp) {
			factories.emplace_back(fp.building, fp.level);
		}
		void add_loyalty_pair(party_loyalty_reader const& lp) {
			party_loyalty.emplace_back(lp.ideology, lp.loyalty_value);
		}
	};

	void merge_province_history_blocks(province_history_block& base_block, province_history_block const& new_block) {
		if(is_valid_index(new_block.trade_goods))
			base_block.trade_goods = new_block.trade_goods;
		if(new_block.life_rating)
			base_block.life_rating = new_block.life_rating;
		if(is_valid_index(new_block.owner))
			base_block.owner = new_block.owner;
		if(is_valid_index(new_block.controller))
			base_block.controller = new_block.controller;
		if(is_valid_index(new_block.terrain))
			base_block.terrain = new_block.terrain;

		base_block.add_cores.insert(base_block.add_cores.end(), new_block.add_cores.begin(), new_block.add_cores.end());
		for(auto c : new_block.remove_cores) {
			if(auto fr = std::find(base_block.add_cores.begin(), base_block.add_cores.end(), c); fr != base_block.add_cores.end()) {
				*fr = base_block.add_cores.back();
				base_block.add_cores.pop_back();
			}
		}
		for(auto fa : new_block.factories) {
			if(auto efa =
				std::find_if(base_block.factories.begin(), base_block.factories.end(),
					[tag = fa.first](std::pair<economy::factory_type_tag, int32_t>& p) {return p.first == tag; });
			efa != base_block.factories.end()) {
				efa->second = fa.second;
			} else {
				base_block.factories.push_back(fa);
			}
		}
		if(new_block.fort_level)
			base_block.fort_level = new_block.fort_level;
		if(new_block.naval_base_level)
			base_block.naval_base_level = new_block.naval_base_level;
		if(new_block.railroad_level)
			base_block.railroad_level = new_block.railroad_level;
		if(new_block.colony)
			base_block.colony = new_block.colony;

		for(auto p : new_block.party_loyalty) {
			if(auto epl =
				std::find_if(base_block.party_loyalty.begin(), base_block.party_loyalty.end(),
					[tag = p.first](std::pair<ideologies::ideology_tag, float>& ip) {return ip.first == tag; });
			epl != base_block.party_loyalty.end()) {
				epl->second = p.second;
			} else {
				base_block.party_loyalty.push_back(p);
			}
		}
	}

	inline int discard_section(token_group const*, token_group const*, province_history_environment&) {
		return 0;
	}
	inline std::pair<token_and_type, province_history_block> name_block(token_and_type const& l, association_type, province_history_block&& r) {
		return std::pair<token_and_type, province_history_block>(l, std::move(r));
	}

	struct parsing_environment {
		text_data::text_sequences& text_lookup;

		province_manager& manager;
		modifiers::modifiers_manager& mod_manager;

		parsed_data climate_file;

		parsing_environment(text_data::text_sequences& tl, province_manager& m, modifiers::modifiers_manager& mm) :
			text_lookup(tl), manager(m), mod_manager(mm) {}
	};

	parsing_state::parsing_state(text_data::text_sequences& tl, province_manager& m, modifiers::modifiers_manager& mm) :
		impl(std::make_unique<parsing_environment>(tl, m, mm)) {}
	parsing_state::~parsing_state() {}

	parsing_state::parsing_state(parsing_state&& o) noexcept : impl(std::move(o.impl)) {}

	struct empty_type {
		void add_unknown_key(int) {}
	};

	struct sea_starts {
		parsing_environment& env;
		std::vector<int8_t> marked_as_sea;

		sea_starts(parsing_environment& e) : env(e) {
			marked_as_sea.resize(env.manager.province_container.size());
		}

		void add_sea_start(uint16_t v) {
			marked_as_sea[v] = 1i8;
			//env.manager.province_container.set<province::is_sea>(province_tag(v), true);
		}
	};

	struct default_map_file {
		parsing_environment& env;
		

		default_map_file(parsing_environment& e) : env(e) {}
		

		void discard(int) {}
		void discard_empty(const empty_type&) {}
		void set_province_count(size_t v) {
			env.manager.province_container.resize(int32_t(v));
			env.manager.integer_to_province.resize(v);
		}
		void handle_sea_starts(const sea_starts& s) {
			env.manager.integer_to_province[0] = province_tag(0);
			int32_t current_land = 1;
			int32_t current_sea = env.manager.province_container.size() - 1;
			for(int32_t i = 1; i < env.manager.province_container.size(); ++ i) {
				if(s.marked_as_sea[i] != 0) {
					env.manager.province_container.set<province::is_sea>(province_tag(uint16_t(current_sea)), true);
					env.manager.integer_to_province[i] = province_tag(uint16_t(current_sea));
					current_sea--;
				} else {
					env.manager.integer_to_province[i] = province_tag(uint16_t(current_land));
					current_land++;
				}
			}
			env.manager.first_sea_province = current_sea + 1;
		}
	};

	struct terrain_modifier_parsing_environment {
		text_data::text_sequences& text_lookup;

		province_manager& manager;
		modifiers::modifiers_manager& mod_manager;
		graphics::name_maps& gname_maps;

		terrain_modifier_parsing_environment(text_data::text_sequences& tl, province_manager& m, modifiers::modifiers_manager& mm, graphics::name_maps& nm) :
			text_lookup(tl), manager(m), mod_manager(mm), gname_maps(nm) {}
	};

	struct terrain_color_parsing_environment {
		text_data::text_sequences& text_lookup;

		province_manager& manager;
		modifiers::modifiers_manager& mod_manager;
		
		color_to_terrain_map& terrain_color_map;

		terrain_color_parsing_environment(text_data::text_sequences& tl, province_manager& m, modifiers::modifiers_manager& mm, color_to_terrain_map& tcm) :
			text_lookup(tl), manager(m), mod_manager(mm), terrain_color_map(tcm) {}
	};

	struct color_builder {
		uint32_t current_color = 0;
		graphics::color_rgb color = { 0,0,0 };

		void add_value(int v) {
			switch(current_color) {
				case 0:
					color.r = static_cast<uint8_t>(v);
					break;
				case 1:
					color.g = static_cast<uint8_t>(v);
					break;
				case 2:
					color.b = static_cast<uint8_t>(v);
					break;
				default:
					break;
			}
			++current_color;
		}
	};

	struct color_list_builder {
		std::vector<uint8_t> colors;
		void add_value(uint8_t v) { colors.push_back(v); }
	};

	struct color_assignment {
		terrain_color_parsing_environment& env;

		color_list_builder colors;
		modifiers::provincial_modifier_tag tag;

		color_assignment(terrain_color_parsing_environment& e) : env(e) {}

		void set_type(token_and_type const& t) {
			const auto name = text_data::get_thread_safe_existing_text_handle(env.text_lookup, t.start, t.end);
			tag = tag_from_text(env.mod_manager.named_provincial_modifiers_index, name);
		}
		void discard(int) {}
	};

	inline color_assignment get_color_assignment(token_and_type const &, association_type, color_assignment&& v) {
		return std::move(v);
	}

	struct preparse_terrain_category : public modifiers::modifier_reading_base {
		terrain_modifier_parsing_environment& env;

		preparse_terrain_category(terrain_modifier_parsing_environment& e) : env(e) {}
		void add_color(const color_builder& ) {}
		void discard(int) {}
	};

	struct preparse_terrain_categories {
		terrain_modifier_parsing_environment& env;

		preparse_terrain_categories(terrain_modifier_parsing_environment& e) : env(e) {}

		void add_category(std::pair<token_and_type, preparse_terrain_category>&& p) {
			const auto name = text_data::get_thread_safe_text_handle(env.text_lookup, p.first.start, p.first.end);
			const auto new_mod = modifiers::add_provincial_modifier(name, p.second, env.mod_manager);
			if(auto fr = env.gname_maps.names.find(std::string("GFX_terrainimg_") + std::string(p.first.start, p.first.end)); fr != env.gname_maps.names.end()) {
				env.manager.terrain_graphics.emplace(new_mod, fr->second);
			}
		}

	};

	struct preparse_terrain_modifier_file {
		terrain_modifier_parsing_environment& env;
		preparse_terrain_modifier_file(terrain_modifier_parsing_environment& e) : env(e) {}

		void add_categories(const preparse_terrain_categories&) {}
		void discard(int) {}
	};

	struct preparse_terrain_color_file {
		terrain_color_parsing_environment& env;
		preparse_terrain_color_file(terrain_color_parsing_environment& e) : env(e) {}

		void add_color_assignment(color_assignment const & a) {
			for(auto v : a.colors.colors)
				env.terrain_color_map.data[v] = a.tag;
		}
		void discard(int) {}
	};
	inline int discard_tc_section(token_group const*, token_group const*, terrain_color_parsing_environment&) {
		return 0;
	}
	inline int discard_tm_section(token_group const*, token_group const*, token_and_type const&, terrain_modifier_parsing_environment&) {
		return 0;
	}
	inline int discard_empty_type(const token_and_type&, association_type, const empty_type&) { return 0; }
	inline std::pair<token_and_type, preparse_terrain_category>
		bind_terrain_category(const token_and_type& t, association_type, const preparse_terrain_category& f) {
		return std::pair<token_and_type, preparse_terrain_category>(t, std::move(f));
	}

	struct state_parse {
		parsing_environment& env;
		state_tag tag;

		state_parse(parsing_environment& e) : env(e) {
			tag = e.manager.state_names.emplace_back();
		}

		void add_province(uint16_t v) {
			env.manager.province_container.set<province::state_id>(env.manager.integer_to_province[v], tag);
			env.manager.states_to_province_index.add_to_row(tag, env.manager.integer_to_province[v]);
		}
	};

	struct region_file {
		parsing_environment& env;
		region_file(parsing_environment& e) : env(e) {}
		void add_state(const std::pair<token_and_type, state_tag>& p) {
			const auto name = text_data::get_thread_safe_text_handle(env.text_lookup, p.first.start, p.first.end);
			env.manager.state_names[p.second] = name;
			env.manager.named_states_index.emplace(name, p.second);
		}
	};

	inline std::pair<token_and_type, state_tag>
		bind_state(const token_and_type& t, association_type, const state_parse& f) {

		return std::pair<token_and_type, state_tag>(t, f.tag);
	}

	struct parse_continent : public modifiers::modifier_reading_base {
		parsing_environment& env;
		modifiers::provincial_modifier_tag tag;

		parse_continent(parsing_environment& e) : env(e) {
			tag = e.mod_manager.provincial_modifiers.emplace_back();
			e.mod_manager.provincial_modifiers[tag].id = tag;
		}

		void add_continent_provinces(const std::vector<uint16_t>& v) {
			for(auto i : v) {
				env.manager.province_container.set<province::continent>(env.manager.integer_to_province[i], tag);
			}
		}

		void discard(int) {}
	};

	struct continents_parse_file {
		parsing_environment& env;
		continents_parse_file(parsing_environment& e) : env(e) {}

		void add_continent(const std::pair<token_and_type, modifiers::provincial_modifier_tag>& p) {
			const auto name = text_data::get_thread_safe_text_handle(env.text_lookup, p.first.start, p.first.end);
			env.mod_manager.named_provincial_modifiers_index.emplace(name, p.second);
			env.mod_manager.provincial_modifiers[p.second].name = name;
		}
	};

	inline std::pair<token_and_type, modifiers::provincial_modifier_tag>
		bind_continent(const token_and_type& t, association_type, parse_continent&& f) {

		modifiers::set_provincial_modifier(f.tag, f, f.env.mod_manager);
		return std::pair<token_and_type, modifiers::provincial_modifier_tag>(t, f.tag);
	}

	inline modifiers::provincial_modifier_tag get_or_make_prov_modifier(text_data::text_tag name, modifiers::modifiers_manager& m) {
		const auto existing_tag = m.named_provincial_modifiers_index.find(name);
		if(existing_tag != m.named_provincial_modifiers_index.end()) {
			return existing_tag->second;
		} else {
			const auto ntag = m.provincial_modifiers.emplace_back();
			m.provincial_modifiers[ntag].id = ntag;
			m.provincial_modifiers[ntag].name = name;
			m.named_provincial_modifiers_index.emplace(name, ntag);
			return ntag;
		}
	}
	struct climate_province_values {
		std::vector<uint16_t> values;
		void add_province(uint16_t v) {
			values.push_back(v);
		}
	};

	int add_individual_climate(const token_group* s, const token_group* e, const token_and_type& t, parsing_environment& env);

	struct climate_pre_parse_file {
		parsing_environment& env;
		climate_pre_parse_file(parsing_environment& e) : env(e) {}

		void add_climate(int) {}
	};

	inline std::pair<token_and_type, float> full_to_tf_pair(const token_and_type& t, association_type, const token_and_type& r) {
		return std::pair<token_and_type, float>(t, token_to<float>(r));
	}
}

MEMBER_DEF(provinces::province_history_block, life_rating, "life_rating");
MEMBER_DEF(provinces::province_history_block, fort_level, "fort");
MEMBER_DEF(provinces::province_history_block, naval_base_level, "naval_base");
MEMBER_DEF(provinces::province_history_block, railroad_level, "railroad");
MEMBER_DEF(provinces::province_history_block, colony, "colony");
MEMBER_FDEF(provinces::province_history_block, discard, "discard");
MEMBER_FDEF(provinces::province_history_block, add_dated_block, "dated_block");
MEMBER_FDEF(provinces::province_history_block, set_trade_goods, "trade_goods");
MEMBER_FDEF(provinces::province_history_block, set_owner, "owner");
MEMBER_FDEF(provinces::province_history_block, set_controller, "controller");
MEMBER_FDEF(provinces::province_history_block, set_terrain, "terrain");
MEMBER_FDEF(provinces::province_history_block, add_core, "add_core");
MEMBER_FDEF(provinces::province_history_block, remove_core, "remove_core");
MEMBER_FDEF(provinces::province_history_block, add_factory_pair, "state_building");
MEMBER_FDEF(provinces::province_history_block, add_loyalty_pair, "party_loyalty");
MEMBER_DEF(provinces::factory_pair_reader, level, "level");
MEMBER_FDEF(provinces::factory_pair_reader, discard, "discard");
MEMBER_FDEF(provinces::factory_pair_reader, set_building, "building");
MEMBER_DEF(provinces::party_loyalty_reader, loyalty_value, "loyalty_value");
MEMBER_FDEF(provinces::party_loyalty_reader, set_ideology, "ideology");


MEMBER_DEF(provinces::color_assignment, colors, "color");
MEMBER_FDEF(provinces::color_assignment, discard, "discard");
MEMBER_FDEF(provinces::color_assignment, set_type, "type");
MEMBER_FDEF(provinces::color_list_builder, add_value, "value");
MEMBER_FDEF(provinces::preparse_terrain_color_file, add_color_assignment, "color_assignment");
MEMBER_FDEF(provinces::preparse_terrain_color_file, discard, "unknown_key");
MEMBER_FDEF(provinces::empty_type, add_unknown_key, "unknown_key");
MEMBER_FDEF(provinces::default_map_file, discard, "unknown_key");
MEMBER_FDEF(provinces::default_map_file, discard_empty, "discard_empty");
MEMBER_FDEF(provinces::default_map_file, set_province_count, "max_provinces");
MEMBER_FDEF(provinces::default_map_file, handle_sea_starts, "sea_starts");
MEMBER_FDEF(provinces::sea_starts, add_sea_start, "add_sea_start");
MEMBER_FDEF(provinces::color_builder, add_value, "color");
MEMBER_FDEF(provinces::preparse_terrain_modifier_file, add_categories, "categories");
MEMBER_FDEF(provinces::preparse_terrain_modifier_file, discard, "unknown_key");
MEMBER_FDEF(provinces::preparse_terrain_categories, add_category, "category");
MEMBER_FDEF(provinces::preparse_terrain_category, add_color, "color");
MEMBER_DEF(provinces::preparse_terrain_category, icon, "icon");
MEMBER_FDEF(provinces::preparse_terrain_category, discard, "discard");
MEMBER_FDEF(provinces::preparse_terrain_category, add_attribute, "attribute");
MEMBER_FDEF(provinces::region_file, add_state, "state");
MEMBER_FDEF(provinces::state_parse, add_province, "province");
MEMBER_FDEF(provinces::continents_parse_file, add_continent, "continent");
MEMBER_FDEF(provinces::parse_continent, discard, "unknown_key");
MEMBER_FDEF(provinces::parse_continent, add_continent_provinces, "provinces");
MEMBER_DEF(provinces::parse_continent, icon, "icon");
MEMBER_FDEF(provinces::parse_continent, add_attribute, "attribute");
MEMBER_FDEF(provinces::climate_pre_parse_file, add_climate, "climate");
MEMBER_FDEF(provinces::climate_province_values, add_province, "value");

namespace provinces {
	BEGIN_DOMAIN(province_history_domain)
		BEGIN_TYPE(party_loyalty_reader)
			MEMBER_ASSOCIATION("ideology", "ideology", token_from_rh)
			MEMBER_ASSOCIATION("loyalty_value", "loyalty_value", value_from_rh<float>)
		END_TYPE
		BEGIN_TYPE(factory_pair_reader)
			MEMBER_ASSOCIATION("building", "building", token_from_rh)
			MEMBER_ASSOCIATION("discard", "upgrade", discard_from_rh)
			MEMBER_ASSOCIATION("level", "level", value_from_rh<int32_t>)
		END_TYPE
		BEGIN_TYPE(province_history_block)
			MEMBER_ASSOCIATION("life_rating", "life_rating", value_from_rh<int16_t>)
			MEMBER_ASSOCIATION("fort", "fort", value_from_rh<int32_t>)
			MEMBER_ASSOCIATION("naval_base", "naval_base", value_from_rh<int32_t>)
			MEMBER_ASSOCIATION("railroad", "railroad", value_from_rh<int32_t>)
			MEMBER_ASSOCIATION("colony", "colony", value_from_rh<int32_t>)
			MEMBER_ASSOCIATION("colony", "colonial", value_from_rh<int32_t>)
			MEMBER_ASSOCIATION("trade_goods", "trade_goods", token_from_rh)
			MEMBER_ASSOCIATION("owner", "owner", token_from_rh)
			MEMBER_ASSOCIATION("controller", "controller", token_from_rh)
			MEMBER_ASSOCIATION("terrain", "terrain", token_from_rh)
			MEMBER_ASSOCIATION("add_core", "add_core", token_from_rh)
			MEMBER_ASSOCIATION("remove_core", "remove_core", token_from_rh)
			MEMBER_TYPE_ASSOCIATION("party_loyalty", "party_loyalty", party_loyalty_reader)
			MEMBER_TYPE_ASSOCIATION("state_building", "state_building", factory_pair_reader)
			MEMBER_TYPE_EXTERN("discard", "revolt", int, discard_section)
			MEMBER_ASSOCIATION("discard", "is_slave", discard_from_rh)
			MEMBER_ASSOCIATION("discard", "set_province_flag", discard_from_rh)
			MEMBER_ASSOCIATION("discard", "clr_province_flag", discard_from_rh)
			MEMBER_VARIABLE_TYPE_ASSOCIATION("dated_block", accept_all, province_history_block, name_block)
		END_TYPE
	END_DOMAIN;

	BEGIN_DOMAIN(default_map_domain)
		BEGIN_TYPE(empty_type)
		MEMBER_VARIABLE_ASSOCIATION("unknown_key", accept_all, discard_from_full)
		MEMBER_VARIABLE_TYPE_ASSOCIATION("unknown_key", accept_all, empty_type, discard_empty_type)
		END_TYPE
		BEGIN_TYPE(sea_starts)
		MEMBER_VARIABLE_ASSOCIATION("add_sea_start", accept_all, value_from_lh<uint16_t>)
		END_TYPE
		BEGIN_TYPE(default_map_file)
		MEMBER_ASSOCIATION("max_provinces", "max_provinces", value_from_rh<uint32_t>)
		MEMBER_TYPE_ASSOCIATION("sea_starts", "sea_starts", sea_starts)
		MEMBER_TYPE_ASSOCIATION("discard_empty", "border_heights", empty_type)
		MEMBER_TYPE_ASSOCIATION("discard_empty", "terrain_sheet_heights", empty_type)
		MEMBER_ASSOCIATION("unknown_key", "definitions", discard_from_rh)
		MEMBER_ASSOCIATION("unknown_key", "provinces", discard_from_rh)
		MEMBER_ASSOCIATION("unknown_key", "positions", discard_from_rh)
		MEMBER_ASSOCIATION("unknown_key", "terrain", discard_from_rh)
		MEMBER_ASSOCIATION("unknown_key", "rivers", discard_from_rh)
		MEMBER_ASSOCIATION("unknown_key", "terrain_definition", discard_from_rh)
		MEMBER_ASSOCIATION("unknown_key", "tree_definition", discard_from_rh)
		MEMBER_ASSOCIATION("unknown_key", "continent", discard_from_rh)
		MEMBER_ASSOCIATION("unknown_key", "adjacencies", discard_from_rh)
		MEMBER_ASSOCIATION("unknown_key", "region", discard_from_rh)
		MEMBER_ASSOCIATION("unknown_key", "region_sea", discard_from_rh)
		MEMBER_ASSOCIATION("unknown_key", "province_flag_sprite", discard_from_rh)
		MEMBER_ASSOCIATION("unknown_key", "tree", discard_from_rh)
		MEMBER_ASSOCIATION("unknown_key", "border_cutoff", discard_from_rh)
		END_TYPE
	END_DOMAIN;

	BEGIN_DOMAIN(preparse_terrain_color_domain)
		BEGIN_TYPE(color_assignment)
		MEMBER_ASSOCIATION("discard", "priority", discard_from_rh)
		MEMBER_ASSOCIATION("discard", "has_texture", discard_from_rh)
		MEMBER_ASSOCIATION("type", "type", token_from_rh)
		MEMBER_TYPE_ASSOCIATION("color", "color", color_list_builder)
		END_TYPE
		BEGIN_TYPE(preparse_terrain_color_file)
		MEMBER_TYPE_EXTERN("unknown_key", "categories", int, discard_tc_section)
		MEMBER_VARIABLE_ASSOCIATION("unknown_key", accept_all, discard_from_full)
		MEMBER_VARIABLE_TYPE_ASSOCIATION("color_assignment", accept_all, color_assignment, get_color_assignment)
		END_TYPE
		BEGIN_TYPE(color_list_builder)
		MEMBER_VARIABLE_ASSOCIATION("value", accept_all, value_from_lh<uint8_t>)
		END_TYPE
		END_DOMAIN;

	BEGIN_DOMAIN(preparse_terrain_modifier_domain)
		BEGIN_TYPE(preparse_terrain_modifier_file)
		MEMBER_TYPE_ASSOCIATION("categories", "categories", preparse_terrain_categories)
		MEMBER_VARIABLE_ASSOCIATION("unknown_key", accept_all, discard_from_full)
		MEMBER_VARIABLE_TYPE_EXTERN("unknown_key", accept_all, int, discard_tm_section)
		END_TYPE
		BEGIN_TYPE(preparse_terrain_categories)
		MEMBER_VARIABLE_TYPE_ASSOCIATION("category", accept_all, preparse_terrain_category, bind_terrain_category)
		END_TYPE
		BEGIN_TYPE(preparse_terrain_category)
		MEMBER_TYPE_ASSOCIATION("color", "color", color_builder)
		MEMBER_ASSOCIATION("icon", "icon", value_from_rh<uint32_t>)
		MEMBER_VARIABLE_ASSOCIATION("attribute", accept_all, full_to_tf_pair)
		MEMBER_ASSOCIATION("discard", "is_water", discard_from_rh)
		END_TYPE
		BEGIN_TYPE(color_builder)
		MEMBER_VARIABLE_ASSOCIATION("color", accept_all, value_from_lh<int>)
		END_TYPE
		END_DOMAIN;

	BEGIN_DOMAIN(read_states_domain)
		BEGIN_TYPE(region_file)
		MEMBER_VARIABLE_TYPE_ASSOCIATION("state", accept_all, state_parse, bind_state)
		END_TYPE
		BEGIN_TYPE(state_parse)
		MEMBER_VARIABLE_ASSOCIATION("province", accept_all, value_from_lh<uint16_t>)
		END_TYPE
		END_DOMAIN;

	BEGIN_DOMAIN(continents_domain)
		BEGIN_TYPE(continents_parse_file)
		MEMBER_VARIABLE_TYPE_ASSOCIATION("continent", accept_all, parse_continent, bind_continent)
		END_TYPE
		BEGIN_TYPE(parse_continent)
		MEMBER_TYPE_ASSOCIATION("provinces", "provinces", std::vector<uint16_t>)
		MEMBER_ASSOCIATION("icon", "icon", value_from_rh<uint32_t>)
		MEMBER_VARIABLE_ASSOCIATION("attribute", accept_all, full_to_tf_pair)
		END_TYPE
		BEGIN_TYPE(std::vector<uint16_t>)
		MEMBER_VARIABLE_ASSOCIATION("this", accept_all, value_from_lh<uint16_t>)
		END_TYPE
		END_DOMAIN;

	BEGIN_DOMAIN(preparse_climate_domain)
		BEGIN_TYPE(climate_pre_parse_file)
		MEMBER_VARIABLE_TYPE_EXTERN("climate", accept_all, int, add_individual_climate)
		END_TYPE
		BEGIN_TYPE(climate_province_values)
		MEMBER_VARIABLE_ASSOCIATION("value", accept_all, value_from_lh<uint16_t>)
		END_TYPE
		END_DOMAIN;

	void read_province_history(world_state& ws, province_tag ps, date_tag target_date, token_group const* start, token_group const* end) {
		province_history_environment env(ws.s, target_date);
		province_history_block result = parse_object<province_history_block, province_history_domain>(start, end, env);

		auto& container = ws.w.province_s.province_state_container;

		for(auto c : result.add_cores)
			add_core(ws, ps, c);
		if(is_valid_index(result.trade_goods))
			container.set<province_state::rgo_production>(ps, result.trade_goods);
		if(is_valid_index(result.terrain))
			container.set<province_state::terrain>(ps, result.terrain);
		if(result.life_rating)
			container.set<province_state::base_life_rating>(ps, *result.life_rating);
		if(is_valid_index(result.owner))
			provinces::silent_set_province_owner(ws, nations::make_nation_for_tag(ws, result.owner), ps);
		if(is_valid_index(result.controller))
			provinces::silent_set_province_controller(ws, nations::make_nation_for_tag(ws, result.controller), ps);
		if(auto si = container.get<province_state::state_instance>(ps); result.colony && is_valid_index(si)) {

			if(*result.colony == 2)
				nations::set_colonial_status(ws, si, nations::colonial_status::colonial);
			else if(*result.colony == 1)
				nations::set_colonial_status(ws, si, nations::colonial_status::protectorate);
			//else if(*result.colony == 0)
				//si.flags = decltype(si.flags)(0);
		}
		if(auto si = container.get<province_state::state_instance>(ps); is_valid_index(si)) {
			auto& factories = ws.w.nation_s.states.get<state::factories>(si);
			for(uint32_t i = 0; i < result.factories.size() && i < state::factories_count; ++i) {
				if(is_valid_index(result.factories[i].first)) {
					factories[i].type = &(ws.s.economy_m.factory_types[result.factories[i].first]);
					factories[i].level = uint16_t(result.factories[i].second);
				}
			}
		}
		if(result.railroad_level)
			container.set<province_state::railroad_level>(ps, uint8_t(*result.railroad_level));
		if(result.fort_level)
			container.set<province_state::fort_level>(ps, uint8_t(*result.fort_level));
		if(result.naval_base_level)
			container.set<province_state::naval_base_level>(ps, uint8_t(*result.naval_base_level));
		for(auto p : result.party_loyalty)
			ws.w.province_s.party_loyalty.get(ps, p.first) = p.second;
	}

	void read_province_histories(world_state& ws, const directory& root, date_tag target_date) {
		const auto history_dir = root.get_directory(u"\\history");
		const auto provinces_dir = history_dir.get_directory(u"\\provinces");

		const auto sub_dirs = provinces_dir.list_directories();
		for(auto& sd : sub_dirs) {
			const auto province_files = sd.list_files(u".txt");
			for(auto& pfile : province_files) {
				auto file_name =  pfile.file_name();
				auto name_break = std::find_if(file_name.begin(), file_name.end(), [](char16_t c) { return c == u' ' || c == u'-'; });
				auto const prov_id = ws.s.province_m.integer_to_province[u16atoui(file_name.begin().operator->(), name_break.operator->())];

				auto fi = pfile.open_file();
				if(fi &&  ws.s.province_m.province_container.is_valid_index(prov_id)) {
					const auto sz = fi->size();
					std::unique_ptr<char[]> parse_data = std::unique_ptr<char[]>(new char[sz]);
					std::vector<token_group> presults;

					fi->read_to_buffer(parse_data.get(), sz);
					parse_pdx_file(presults, parse_data.get(), parse_data.get() + sz);

					read_province_history(
						ws,
						prov_id,
						target_date,
						presults.data(), presults.data() + presults.size());
				}
			}
		}
	}

	int add_individual_climate(const token_group* s, const token_group* e, const token_and_type& t, parsing_environment& env) {
		const auto name = text_data::get_thread_safe_text_handle(env.text_lookup, t.start, t.end);
		const auto fr = env.mod_manager.named_provincial_modifiers_index.find(name);
		if(fr == env.mod_manager.named_provincial_modifiers_index.end()) {
			modifiers::parse_provincial_modifier(name, env.mod_manager, s, e);
		} else {
			const auto vals = parse_object<climate_province_values, preparse_climate_domain>(s, e, env);
			for(auto v : vals.values)
				env.manager.province_container.set<province::climate>(env.manager.integer_to_province[v], fr->second);
		}
		return 0;
	}

	void read_default_map_file(
		parsing_state& state,
		const directory& source_directory) {

		const auto map_dir = source_directory.get_directory(u"\\map");
		parsed_data main_results;

		const auto fi = map_dir.open_file(u"default.map");

		if(fi) {
			const auto sz = fi->size();
			main_results.parse_data = std::unique_ptr<char[]>(new char[sz]);

			fi->read_to_buffer(main_results.parse_data.get(), sz);
			parse_pdx_file(main_results.parse_results, main_results.parse_data.get(), main_results.parse_data.get() + sz);

			if(main_results.parse_results.size() > 0) {
				parse_object<default_map_file, default_map_domain>(
					&main_results.parse_results[0],
					&main_results.parse_results[0] + main_results.parse_results.size(),
					*state.impl);
			}
		}
	}

	void read_terrain_modifiers(
		text_data::text_sequences& text,
		province_manager& pm,
		modifiers::modifiers_manager& mm,
		graphics::name_maps& gname_maps,
		const directory& source_directory) {

		const auto map_dir = source_directory.get_directory(u"\\map");
		parsed_data main_results;

		terrain_modifier_parsing_environment tstate(text, pm, mm, gname_maps);

		const auto fi = map_dir.open_file(u"terrain.txt");

		if(fi) {
			const auto sz = fi->size();
			main_results.parse_data = std::unique_ptr<char[]>(new char[sz]);

			fi->read_to_buffer(main_results.parse_data.get(), sz);
			parse_pdx_file(main_results.parse_results, main_results.parse_data.get(), main_results.parse_data.get() + sz);

			if(main_results.parse_results.size() > 0) {
				parse_object<preparse_terrain_modifier_file, preparse_terrain_modifier_domain>(
					&main_results.parse_results[0],
					&main_results.parse_results[0] + main_results.parse_results.size(),
					tstate);
			}
		}
	}

	color_to_terrain_map read_terrain_colors(
		text_data::text_sequences& text,
		province_manager& pm,
		modifiers::modifiers_manager& mm,
		const directory& source_directory) {

		const auto map_dir = source_directory.get_directory(u"\\map");
		parsed_data main_results;
		color_to_terrain_map result_map;

		terrain_color_parsing_environment tstate(text, pm, mm, result_map);

		const auto fi = map_dir.open_file(u"terrain.txt");

		if(fi) {
			const auto sz = fi->size();
			main_results.parse_data = std::unique_ptr<char[]>(new char[sz]);

			fi->read_to_buffer(main_results.parse_data.get(), sz);
			parse_pdx_file(main_results.parse_results, main_results.parse_data.get(), main_results.parse_data.get() + sz);

			if(main_results.parse_results.size() > 0) {
				parse_object<preparse_terrain_color_file, preparse_terrain_color_domain>(
					&main_results.parse_results[0],
					&main_results.parse_results[0] + main_results.parse_results.size(),
					tstate);
			}
		}

		return result_map;
	}

	void read_states(
		parsing_state& state,
		const directory& source_directory) {

		const auto map_dir = source_directory.get_directory(u"\\map");
		parsed_data main_results;

		const auto fi = map_dir.open_file(u"region.txt");

		if(fi) {
			const auto sz = fi->size();
			main_results.parse_data = std::unique_ptr<char[]>(new char[sz]);

			fi->read_to_buffer(main_results.parse_data.get(), sz);
			parse_pdx_file(main_results.parse_results, main_results.parse_data.get(), main_results.parse_data.get() + sz);

			if(main_results.parse_results.size() > 0) {
				parse_object<region_file, read_states_domain>(
					&main_results.parse_results[0],
					&main_results.parse_results[0] + main_results.parse_results.size(),
					*state.impl);
			}
		}
	}

	void read_continents(
		parsing_state& state,
		const directory& source_directory) {

		const auto map_dir = source_directory.get_directory(u"\\map");
		parsed_data main_results;

		const auto fi = map_dir.open_file(u"continent.txt");

		if(fi) {
			const auto sz = fi->size();
			main_results.parse_data = std::unique_ptr<char[]>(new char[sz]);

			fi->read_to_buffer(main_results.parse_data.get(), sz);
			parse_pdx_file(main_results.parse_results, main_results.parse_data.get(), main_results.parse_data.get() + sz);

			if(main_results.parse_results.size() > 0) {
				parse_object<continents_parse_file, continents_domain>(
					&main_results.parse_results[0],
					&main_results.parse_results[0] + main_results.parse_results.size(),
					*state.impl);
			}
		}
	}

	void read_climates(
		parsing_state& state,
		const directory& source_directory) {

		const auto map_dir = source_directory.get_directory(u"\\map");
		auto& main_results = state.impl->climate_file;

		const auto fi = map_dir.open_file(u"climate.txt");

		if(fi) {
			const auto sz = fi->size();
			main_results.parse_data = std::unique_ptr<char[]>(new char[sz]);

			fi->read_to_buffer(main_results.parse_data.get(), sz);
			parse_pdx_file(main_results.parse_results, main_results.parse_data.get(), main_results.parse_data.get() + sz);

			if(main_results.parse_results.size() > 0) {
				parse_object<climate_pre_parse_file, preparse_climate_domain>(
					&main_results.parse_results[0],
					&main_results.parse_results[0] + main_results.parse_results.size(),
					*state.impl);
			}
		}
	}
	boost::container::flat_map<uint32_t, int32_t> read_province_definition_file(directory const & source_directory) {
		boost::container::flat_map<uint32_t, int32_t> t;

		const auto map_dir = source_directory.get_directory(u"\\map");
		const auto fi = map_dir.open_file(u"definition.csv");

		if(fi) {
			const auto sz = fi->size();
			const auto parse_data = std::unique_ptr<char[]>(new char[sz]);

			char* position = parse_data.get();
			fi->read_to_buffer(position, sz);
			const char* cpos = position;

			if(sz != 0 && position[0] == '#')
				cpos = csv_advance_to_next_line(cpos, parse_data.get() + sz);

			while(cpos < parse_data.get() + sz) {
				cpos = parse_fixed_amount_csv_values<4>(cpos, parse_data.get() + sz, ';',
					[&t](std::pair<char const*, char const*> const* values) {
					if(is_integer(values[0].first, values[0].second)) {
						t.emplace(rgb_to_prov_index(
							uint8_t(parse_int(values[1].first, values[1].second)),
							uint8_t(parse_int(values[2].first, values[2].second)),
							uint8_t(parse_int(values[3].first, values[3].second))),
							parse_int(values[0].first, values[0].second));
					}
				});
			}

		}
		return t;
	}

	tagged_vector<uint8_t, province_tag> load_province_terrain_data(province_manager& m, directory const& root) {
		const auto map_dir = root.get_directory(u"\\map");

		auto terrain_peek = map_dir.peek_file(u"terrain.png");
		if(terrain_peek) {
			auto fi = terrain_peek->open_file();
			if(fi) {
				const auto sz = fi->size();
				std::unique_ptr<char[]> file_data = std::unique_ptr<char[]>(new char[sz]);
				fi->read_to_buffer(file_data.get(), sz);

				int32_t terrain_width = 0;
				int32_t terrain_height = 0;
				int32_t channels = 1;
				const auto raw_data = SOIL_load_image_from_memory((unsigned char*)(file_data.get()), static_cast<int32_t>(sz), &terrain_width, &terrain_height, &channels, 1);

				if(terrain_height != m.province_map_height || terrain_width != m.province_map_width)
					std::abort();

				const auto t_vector = generate_province_terrain(m.province_container.size(), m.province_map_data.data(), (uint8_t const*)(raw_data), terrain_height, terrain_width);

				SOIL_free_image_data(raw_data);

				return t_vector;
			}
		} else {
			if(terrain_peek) {
				terrain_peek = map_dir.peek_file(u"terrain.bmp");
				auto fi = terrain_peek->open_file();
				if(fi) {
					const auto sz = fi->size();
					std::unique_ptr<char[]> file_data = std::unique_ptr<char[]>(new char[sz]);
					fi->read_to_buffer(file_data.get(), sz);

					BITMAPFILEHEADER header;
					memcpy(&header, file_data.get(), sizeof(BITMAPFILEHEADER));
					BITMAPINFOHEADER info_header;
					memcpy(&info_header, file_data.get() + sizeof(BITMAPFILEHEADER), sizeof(BITMAPINFOHEADER));

					if(info_header.biHeight != m.province_map_height || info_header.biWidth != m.province_map_width)
						std::abort();

					return generate_province_terrain_inverse(m.province_container.size(), m.province_map_data.data(), (uint8_t const*)(file_data.get() + header.bfOffBits), info_header.biHeight, info_header.biWidth);
				}
			}
		}

		return tagged_vector<uint8_t, province_tag>();
	}

	void load_province_map_data(province_manager& m, directory const& root) {
		const auto map_dir = root.get_directory(u"\\map");
		
		auto map_peek = map_dir.peek_file(u"provinces.png");
		if(!map_peek)
			map_peek = map_dir.peek_file(u"provinces.bmp");
		if(map_peek) {
			auto fi = map_peek->open_file();
			if(fi) {
				const auto sz = fi->size();
				std::unique_ptr<char[]> file_data = std::unique_ptr<char[]>(new char[sz]);
				fi->read_to_buffer(file_data.get(), sz);

				int32_t channels = 3;
				const auto raw_data = SOIL_load_image_from_memory((unsigned char*)(file_data.get()), static_cast<int32_t>(sz), &m.province_map_width, &m.province_map_height, &channels, 3);
				m.province_map_data.resize(static_cast<size_t>(m.province_map_width * m.province_map_height));

				const auto color_mapping = read_province_definition_file(root);

				const auto last = m.province_map_width * m.province_map_height - 1;
				uint32_t previous_color_index = provinces::rgb_to_prov_index(raw_data[last * 3 + 0], raw_data[last * 3 + 1], raw_data[last * 3 + 2]);
				province_tag prev_result = province_tag(0);
				if(auto it = color_mapping.find(previous_color_index); it != color_mapping.end()) {
					prev_result = m.integer_to_province[it->second];
					m.province_map_data[static_cast<size_t>(last)] = uint16_t(to_index(prev_result));
				}

				for(int32_t t = m.province_map_width * m.province_map_height - 2; t >= 0; --t) {
					uint32_t color_index = provinces::rgb_to_prov_index(raw_data[t * 3 + 0], raw_data[t * 3 + 1], raw_data[t * 3 + 2]);
					if(color_index == previous_color_index) {
						m.province_map_data[static_cast<size_t>(t)] = uint16_t(to_index(prev_result));
					} else {
						previous_color_index = color_index;
						if(auto it = color_mapping.find(color_index); it != color_mapping.end())
							m.province_map_data[static_cast<size_t>(t)] = uint16_t(to_index(m.integer_to_province[it->second]));
						else
							m.province_map_data[static_cast<size_t>(t)] = 0ui16;
						prev_result = province_tag(m.province_map_data[static_cast<size_t>(t)]);
					}
				}


				SOIL_free_image_data(raw_data);
			}
		}

		
	}

	tagged_vector<uint8_t, province_tag> generate_province_terrain_inverse(size_t province_count, uint16_t const* province_map_data, uint8_t const* terrain_color_map_data, int32_t height, int32_t width) {
		tagged_vector<uint8_t, province_tag> terrain_out;
		tagged_vector<int32_t, province_tag> count;

		terrain_out.resize(province_count);
		count.resize(province_count);


		for(int32_t j = height - 1; j >= 0; --j) {
			for(int32_t i = width - 1; i >= 0; --i) {
				auto this_province = province_tag(province_map_data[j * width + i]);
				const auto t_color = terrain_color_map_data[(height - j - 1) * width + i];
				if(count[this_province] == 0) {
					terrain_out[this_province] = t_color;
					count[this_province] = 1;
				} else if(terrain_out[this_province] == t_color) {
					count[this_province] += 1;
				} else {
					count[this_province] -= 1;
				}
			}
		}

		return terrain_out;
	}

	tagged_vector<uint8_t, province_tag> generate_province_terrain(size_t province_count, uint16_t const* province_map_data, uint8_t const* terrain_color_map_data, int32_t height, int32_t width) {
		tagged_vector<uint8_t, province_tag> terrain_out;
		tagged_vector<int32_t, province_tag> count;

		terrain_out.resize(province_count);
		count.resize(province_count);
		
		for(int32_t i = height  * width - 1; i >= 0; --i) {
			auto this_province = province_tag(province_map_data[i]);
			const auto t_color = terrain_color_map_data[i];
			if(count[this_province] == 0) {
				terrain_out[this_province] = t_color;
				count[this_province] = 1;
			} else if(terrain_out[this_province] == t_color) {
				count[this_province] += 1;
			} else {
				count[this_province] -= 1;
			}
			
		}

		return terrain_out;
	}

	void assign_terrain_color(provinces_state& m, tagged_vector<uint8_t, province_tag> const & terrain_colors, color_to_terrain_map const & terrain_map) {
		int32_t max_province = static_cast<int32_t>(m.province_state_container.size()) - 1;
		for(int32_t i = max_province; i >= 0; --i) {
			const auto this_province = province_tag(static_cast<province_tag::value_base_t>(i));
			const auto this_t_color = terrain_colors[this_province];
			m.province_state_container.set<province_state::terrain>(this_province, terrain_map.data[this_t_color]);
		}
	}

	std::map<province_tag, boost::container::flat_set<province_tag>> generate_map_adjacencies(uint16_t const* province_map_data, int32_t height, int32_t width) {
		std::map<province_tag, boost::container::flat_set<province_tag>> result;

		for(int32_t j = height - 2; j >= 0; --j) {
			for(int32_t i = width - 2; i >= 0; --i) {
				const auto current_prov = province_map_data[i + j * width];
				const auto rightwards = province_map_data[(i + 1) + j * width];
				const auto downwards = province_map_data[i + (j + 1) * width];

				if(rightwards != current_prov) {
					result[province_tag(current_prov)].insert(province_tag(rightwards));
					result[province_tag(rightwards)].insert(province_tag(current_prov));
				}
				if(downwards != current_prov) {
					result[province_tag(current_prov)].insert(province_tag(downwards));
					result[province_tag(downwards)].insert(province_tag(current_prov));
				}
			}
		}

		for(int32_t j = height - 2; j >= 0; --j) {
			const auto current_prov = province_map_data[(width - 1) + j * width];

			const auto rightwards = province_map_data[0 + j * width];
			const auto downwards = province_map_data[(width - 1) + (j + 1) * width];

			if(rightwards != current_prov) {
				result[province_tag(current_prov)].insert(province_tag(rightwards));
				result[province_tag(rightwards)].insert(province_tag(current_prov));
			}
			if(downwards != current_prov) {
				result[province_tag(current_prov)].insert(province_tag(downwards));
				result[province_tag(downwards)].insert(province_tag(current_prov));
			}
		}

		return result;
	}

	void read_adjacnencies_file(std::map<province_tag, boost::container::flat_set<province_tag>>& adj_map, directory const& root, scenario::scenario_manager& s) {
		const auto map_dir = root.get_directory(u"\\map");
		const auto fi = map_dir.open_file(u"adjacencies.csv");

		if(fi) {
			const auto sz = fi->size();
			const auto parse_data = std::unique_ptr<char[]>(new char[sz]);

			char* position = parse_data.get();
			fi->read_to_buffer(position, sz);
			char const* cpos = position;

			if(sz != 0 && position[0] == '#')
				cpos = csv_advance_to_next_line(cpos, parse_data.get() + sz);

			while(cpos < parse_data.get() + sz) {
				cpos = parse_fixed_amount_csv_values<6>(cpos, parse_data.get() + sz, ';',
					[&adj_map, &s](std::pair<char const*, char const*> const* values) {
					const auto prov_a = s.province_m.integer_to_province[parse_int(values[0].first, values[0].second)];
					const auto prov_b = s.province_m.integer_to_province[parse_int(values[1].first, values[1].second)];
					

					if(is_fixed_token_ci(values[2].first, values[2].second, "impassable")) {
						if(prov_b != province_tag(0)) {
							adj_map[prov_b].erase(prov_a);
							adj_map[prov_a].erase(prov_b);
						} else {
							for(auto a : adj_map[prov_a]) {
								adj_map[a].erase(prov_a);
							}
							adj_map[prov_a].clear();
						}
					} else if(is_fixed_token_ci(values[2].first, values[2].second, "canal")) {
						const int32_t canal_index = parse_int(values[4].first, values[4].second) - 1;
						if(static_cast<size_t>(canal_index) >= s.province_m.canals.size())
							s.province_m.canals.resize(static_cast<size_t>(canal_index + 1));
						if(canal_index >= 0) {
							const auto prov_through = province_tag(uint16_t(parse_int(values[3].first, values[3].second)));
							s.province_m.canals[static_cast<size_t>(canal_index)] = std::make_tuple(prov_a, prov_b, text_data::get_thread_safe_text_handle(s.gui_m.text_data_sequences, values[5].first, values[5].second), prov_through);
						}
					} else {
						adj_map[prov_a].insert(prov_b);
						adj_map[prov_b].insert(prov_a);
					}
				});
			}
		}
	}

	void make_lakes(std::map<province_tag, boost::container::flat_set<province_tag>>& adj_map, province_manager& m) {
		m.province_container.set<province::is_lake>(province_tag(0), true);
		m.province_container.set<province::is_sea>(province_tag(0), true);

		for(auto adj_p : adj_map[province_tag(0)])
			adj_map[adj_p].erase(province_tag(0));
		adj_map[province_tag(0)].clear();

		for(uint16_t i = 1; i < m.province_container.size(); ++i) {

			if(m.province_container.get<province::is_sea>(province_tag(i))) {
				bool is_lake = true;
				for(auto adj_p : adj_map[province_tag(i)]) {
					if(m.province_container.get<province::is_sea>(adj_p))
						is_lake = false;
				}
				if(is_lake) {
					m.province_container.set<province::is_lake>(province_tag(i), true);
					for(auto adj_p : adj_map[province_tag(i)])
						adj_map[adj_p].erase(province_tag(i));
					adj_map[province_tag(i)].clear();
				}
			}
		}
	}

	void populate_borders(provinces::province_manager& province_m) {
		const auto wblocks = (province_m.province_map_width + graphics::block_size - 1) / graphics::block_size;
		const auto hblocks = (province_m.province_map_height + graphics::block_size - 1) / graphics::block_size;

		province_m.borders.borders.resize(wblocks * hblocks);

		concurrency::parallel_for(0, hblocks, 1, [&province_m, wblocks](int32_t j) {
			for(int32_t i = 0; i < wblocks; ++i) {
				province_m.borders.borders[i + j * wblocks] = 
					graphics::create_border_block_data(province_m, i, j, province_m.province_map_data.data(), province_m.province_map_width, province_m.province_map_height);
			}
		});
	}

	void make_adjacency(std::map<province_tag, boost::container::flat_set<province_tag>>& adj_map, province_manager& m) {
		m.same_type_adjacency.expand_rows(static_cast<uint32_t>(m.province_container.size()));
		m.coastal_adjacency.expand_rows(static_cast<uint32_t>(m.province_container.size()));
		
		concurrency::parallel_invoke([&adj_map, &m]() {
			for(auto const& adj_set : adj_map) {
				if(m.province_container.get<province::is_sea>(adj_set.first)) {
					for(auto oprov : adj_set.second) {
						if(m.province_container.get<province::is_sea>(oprov)) {
							m.same_type_adjacency.add_to_row(adj_set.first, oprov);
						} else {
							m.coastal_adjacency.add_to_row(adj_set.first, oprov);
							m.province_container.set<province::is_coastal>(adj_set.first, true);
						}
					}
				} else {
					for(auto oprov : adj_set.second) {
						if(m.province_container.get<province::is_sea>(oprov)) {
							m.coastal_adjacency.add_to_row(adj_set.first, oprov);
							m.province_container.set<province::is_coastal>(adj_set.first, true);
						} else {
							m.same_type_adjacency.add_to_row(adj_set.first, oprov);
						}
					}
				}
			}
		}, [&m]() {
			populate_borders(m);
		});
	}

	void calculate_province_areas(province_manager& m, float top_latitude, float bottom_latitude) {
		float surface_area_x_2pi_x_percent_of_width = 510'072'000.0f * 2.0f * 3.1415926f / float(m.province_map_width);
		float quarter_circumfrance = float(m.province_map_width) / 4.0f;
		int32_t half_height = m.province_map_height / 2;

		float lat_step = (bottom_latitude - top_latitude) / float(m.province_map_height);
		float long_step = 6.28318530718f / float(m.province_map_width);
		float top_lat = top_latitude;


		m.province_container.for_each([&m](province_tag p) {
			m.province_container.get<province::centroid>(p).setZero();
			m.province_container.get<province::centroid_2d>(p).setZero();
		});

		for(int32_t y = 0; y < m.province_map_height; ++y) {
			float pixel_area = surface_area_x_2pi_x_percent_of_width *
				(abs(sin((float(abs(y - half_height)) * 3.1415926f) / (quarter_circumfrance * 2.0f)) - sin((float(abs(y + 1 - half_height)) * 3.1415926f) / (quarter_circumfrance * 2.0f))));
			for(int32_t x = 0; x < m.province_map_width; ++x) {
				auto province_id = m.province_map_data[uint32_t(x + y * m.province_map_width)];
				m.province_container.get<province::area>(province_tag(province_id)) += float(pixel_area);

				const float vx_pos = x * long_step;
				const float vy_pos = y * lat_step + top_lat;
				const float cos_vy = cos(vy_pos) * pixel_area;
				const float sin_vy = sin(vy_pos) * pixel_area;

				auto& centroid = m.province_container.get<province::centroid>(province_tag(province_id));
				centroid[0] += cos(vx_pos) * cos_vy;
				centroid[1] += sin(vx_pos) * cos_vy;
				centroid[2] += sin_vy;
			}
		}

		m.province_container.for_each([&m, top_lat, lat_step, long_step](province_tag p) {
			auto& cen = m.province_container.get<province::centroid>(p);

			cen.normalize();
			auto const pr = graphics::map_coordinate_from_globe(cen,
				top_lat, lat_step, 0.0f, long_step, m.province_map_width, m.province_map_height);

			m.province_container.set<province::centroid_2d>(p, Eigen::Vector2f(pr.first, pr.second));
		});
	}

	void set_base_rgo_size(world_state& ws) {
		ws.w.province_s.province_state_container.for_each([&ws](province_tag p) {
			auto size = ws.s.province_m.province_container.get<province::area>(p);
			auto& rgo_size = ws.w.province_s.province_state_container.get<province_state::rgo_size>(p);
			if(size <= 145'000.0f)
				rgo_size = 1ui8;
			else if(size <= 580'000.0f)
				rgo_size = 2ui8;
			else if(size <= 1'300'000.0f)
				rgo_size = 3ui8;
			else
				rgo_size = 4ui8;

			auto rgo_production = ws.w.province_s.province_state_container.get<province_state::rgo_production>(p);
			if(is_valid_index(rgo_production)) {
				if((ws.s.economy_m.goods[rgo_production].flags & economy::good_definition::mined) != 0) {
					for(uint32_t i = 0; i < std::extent_v<decltype(ws.s.economy_m.rgo_mine.workers)>; ++i) {
						if(is_valid_index(ws.s.economy_m.rgo_farm.workers[i].type)) {
							auto worker_population = ws.w.province_s.province_demographics.get(p, population::to_demo_tag(ws, ws.s.economy_m.rgo_mine.workers[i].type));
							auto size_to_support = int32_t(std::ceil(float(worker_population) /
								(ws.s.economy_m.rgo_mine.workers[i].amount * ws.s.economy_m.rgo_mine.workforce * (ws.w.province_s.modifier_values.get<modifiers::provincial_offsets::mine_rgo_size>(p) + 1.0f))));

							rgo_size = uint8_t(std::max(size_to_support, int32_t(rgo_size)));
						}
					}
				} else {
					for(uint32_t i = 0; i < std::extent_v<decltype(ws.s.economy_m.rgo_farm.workers)>; ++i) {
						if(is_valid_index(ws.s.economy_m.rgo_farm.workers[i].type)) {
							auto worker_population = ws.w.province_s.province_demographics.get(p, population::to_demo_tag(ws, ws.s.economy_m.rgo_farm.workers[i].type));
							auto size_to_support = int32_t(std::ceil(float(worker_population) /
								(ws.s.economy_m.rgo_farm.workers[i].amount * ws.s.economy_m.rgo_farm.workforce * (ws.w.province_s.modifier_values.get<modifiers::provincial_offsets::farm_rgo_size>(p) + 1.0f))));

							rgo_size = uint8_t(std::max(size_to_support, int32_t(rgo_size)));
						}
					}
				}

				rgo_size = uint8_t(std::clamp((int32_t(rgo_size) * 3) / 2, 1, 255));
			}
		});

	}
}
