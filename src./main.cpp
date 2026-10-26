#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <set>
#include <utility>
#include <vector>

using namespace geode::prelude;

static constexpr float BLOCK = 30.f;

class $modify(AutoDecoUI, EditorUI) {
	bool init(LevelEditorLayer* lel) {
		if (!EditorUI::init(lel)) return false;

		auto spr = ButtonSprite::create("Deco", 40, true, "bigFont.fnt", "GJ_button_04.png", 30.f, 0.6f);
		auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(AutoDecoUI::onAutoDeco));
		btn->setID("auto-deco-button"_spr);

		auto menu = CCMenu::create();
		menu->setID("auto-deco-menu"_spr);
		menu->addChild(btn);
		auto win = CCDirector::get()->getWinSize();
		menu->setPosition({win.width - 40.f, win.height - 90.f});
		btn->setPosition({0.f, 0.f});
		this->addChild(menu, 100);

		return true;
	}

	void onAutoDeco(CCObject*) {
		auto objs = this->getSelectedObjects();
		if (!objs || objs->count() == 0) {
			FLAlertLayer::create("Auto Deco", "Pilih dulu objek yang mau didekorasi.", "OK")->show();
			return;
		}

		auto mod = Mod::get();
		int decoID = static_cast<int>(mod->getSettingValue<int64_t>("deco-id"));
		auto style = mod->getSettingValue<std::string>("deco-style");
		bool back = mod->getSettingValue<bool>("deco-layer-back");
		bool skip = mod->getSettingValue<bool>("deco-skip-occupied");

		// offset (dalam satuan blok) menurut gaya
		std::vector<std::pair<int, int>> offsets;
		if (style == "Atas") {
			offsets = {{0, 1}};
		} else if (style == "Bawah") {
			offsets = {{0, -1}};
		} else if (style == "Kiri-Kanan") {
			offsets = {{-1, 0}, {1, 0}};
		} else if (style == "Sudut") {
			offsets = {{-1, 1}, {1, 1}, {-1, -1}, {1, -1}};
		} else {
			offsets = {{0, 1}, {0, -1}, {-1, 0}, {1, 0}};
		}

		// kumpulkan posisi objek terpilih (grid 30 unit)
		std::vector<CCPoint> positions;
		std::set<std::pair<int, int>> occupied;
		for (auto obj : CCArrayExt<GameObject*>(objs)) {
			auto p = obj->getPosition();
			positions.push_back(p);
			occupied.insert({
				static_cast<int>(std::round(p.x / BLOCK)),
				static_cast<int>(std::round(p.y / BLOCK))
			});
		}

		std::set<std::pair<int, int>> placed;
		int count = 0;
		for (auto const& p : positions) {
			int gx = static_cast<int>(std::round(p.x / BLOCK));
			int gy = static_cast<int>(std::round(p.y / BLOCK));
			for (auto const& [dx, dy] : offsets) {
				std::pair<int, int> cell{gx + dx, gy + dy};
				if (skip && occupied.count(cell)) continue;
				if (placed.count(cell)) continue;
				placed.insert(cell);

				auto pos = CCPoint{cell.first * BLOCK, cell.second * BLOCK};
				auto deco = m_editorLayer->createObject(decoID, pos, false);
				if (deco && back) {
					deco->m_zLayer = ZLayer::B1;
					deco->m_zOrder = -1;
				}
				count++;
			}
		}

		Notification::create(
			fmt::format("Auto Deco: {} dekorasi ditaruh", count),
			NotificationIcon::Success
		)->show();
	}
};
