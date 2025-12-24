#include "Geode/cocos/base_nodes/CCNode.h"
#include <Geode/Geode.hpp>
#include <Geode/Loader.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/loader/Mod.hpp>

#include <Geode/modify/UILayer.hpp>
#include <Geode/modify/EndLevelLayer.hpp>

using namespace geode::prelude;

/// Wrapper event around `listenForSettingChanges`
class ToggleCBFEvent : public Event {
protected:
	bool m_settingVal;
public:
	ToggleCBFEvent(bool settingVal) : m_settingVal(settingVal) {}
	bool getSettingVal() const { return m_settingVal; }
};

class ToggleCBFEventFilter : public EventFilter<ToggleCBFEvent> {
public:
	using Callback = ListenerResult(bool);

	ListenerResult handle(std::function<Callback> callback, ToggleCBFEvent* ev) {
		auto on = !ev->getSettingVal();
		return callback(on);
	}
	ToggleCBFEventFilter(CCNode* invoker) {} // I need to pass "invoker" as a constructor parameter so "this->addEventListener" works
};

$execute {
	auto cbf = Loader::get()->getLoadedMod("syzzi.click_between_frames");
	if (!cbf) return;
	listenForSettingChanges<bool>("soft-toggle", [](bool val) {
		ToggleCBFEvent(val).post();
	}, cbf);
}

class $modify(CBFIndUILayer, UILayer) {
	bool init(GJBaseGameLayer* layer) {
		if (!UILayer::init(layer)) return false;

		auto cbf = Loader::get()->getLoadedMod("syzzi.click_between_frames");
		bool isCBFOn = cbf && !cbf->getSettingValue<bool>("soft-toggle");

		/* Wish I didn't had to copy-paste the exact same setup code on both if blocks, since that makes it
		harder to change stuff later on, but welp, that's all I can do ;-; */
		if (Mod::get()->getSettingValue<bool>("show-as-image")) {
			auto indicator = CCSprite::create("cbf.png"_spr);

			indicator->setScale(.2f);
			positionIndicator(indicator);
			indicator->setOpacity(50);

			indicator->setID("cbf_indicator"_spr);

			indicator->setVisible(isCBFOn);

			this->addChild(indicator);
			this->addEventListener<ToggleCBFEventFilter>([indicator](bool on) {
				indicator->setVisible(on);
				return ListenerResult::Stop;
			});
		} else {
			auto indicator = CCLabelBMFont::create("CBF", "bigFont.fnt");

			indicator->setScale(.3f);
			positionIndicator(indicator);
			indicator->setOpacity(50);

			indicator->setID("cbf_indicator"_spr);

			indicator->setVisible(isCBFOn);

			this->addChild(indicator);
			this->addEventListener<ToggleCBFEventFilter>([indicator](bool on) {
				indicator->setVisible(on);
				return ListenerResult::Stop;
			});
		}

		return true;
	}

	void positionIndicator(CCNode* indicator) {
		auto alignment = Mod::get()->getSettingValue<std::string>("alignment");
		auto winSize = CCDirector::sharedDirector()->getWinSize();

		if (alignment == "Top-Left") {
			indicator->setPosition({ 0, winSize.height });
			indicator->setAnchorPoint({ 0, 1 });
		} else if (alignment == "Top-Right") {
			indicator->setPosition({ winSize.width, winSize.height });
			indicator->setAnchorPoint({ 1, 1 });
		} else if (alignment == "Bottom-Left") {
			indicator->setPosition({ 0, 0 });
			indicator->setAnchorPoint({ 0, 0 });
		} else if (alignment == "Bottom-Right") {
			indicator->setPosition({ winSize.width, 0 });
			indicator->setAnchorPoint({ 1, 0 });
		}
	}
};

class $modify(CBFIndEndLevelLayer, EndLevelLayer) {
	void customSetup() {
		EndLevelLayer::customSetup();

		auto cbf = Loader::get()->getLoadedMod("syzzi.click_between_frames");
		if (!cbf || cbf->getSettingValue<bool>("soft-toggle")) return;

		// CBF logo nearby the level complete text
		auto logo = CCSprite::create("cbf.png"_spr);

		logo->setScale(.3f);
		logo->setPosition(CCDirector::sharedDirector()->getWinSize() / 2 + ccp(144.f, 66.f));
		logo->setRotation(-12.f);
		logo->setAnchorPoint({ .5f, .5f });
		logo->setZOrder(16);
		logo->setID("cbf_logo"_spr);

		// Edit the level complete label to say a custom message
		std::string str = Mod::get()->getSettingValue<std::string>("lvl-complete-string");

		auto completeMsg = m_mainLayer->getChildByID("complete-message");
		if (!completeMsg) return;

		if (auto completeMsgArea = typeinfo_cast<TextArea*>(completeMsg)) {
			completeMsgArea->setString(str);
			completeMsgArea->setScale(.7f);
		} else if (auto completeMsgLabel = typeinfo_cast<CCLabelBMFont*>(completeMsg)) {
			completeMsgLabel->setString(str.c_str());
			completeMsgLabel->setScale(.7f);
		}

		m_mainLayer->addChild(logo);
	}
};