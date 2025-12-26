#include <Geode/Geode.hpp>
#include <Geode/Loader.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/loader/Mod.hpp>

#include <Geode/modify/UILayer.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

static bool g_levelFinished = false;
static bool g_isCBFOn = false;

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
		if (g_levelFinished) return ListenerResult::Stop;

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

class $modify(CBFIndPlayLayer, PlayLayer) {
	void levelComplete() {
		PlayLayer::levelComplete();
		g_levelFinished = true;
	}
};

class $modify(CBFIndUILayer, UILayer) {
	bool init(GJBaseGameLayer* layer) {
		if (!UILayer::init(layer)) return false;
		g_levelFinished = false; // Chances are the level's probably not finished yet so this is safe

		auto cbf = Loader::get()->getLoadedMod("syzzi.click_between_frames");
		g_isCBFOn = cbf && !cbf->getSettingValue<bool>("soft-toggle");

		/* Wish I didn't had to copy-paste the exact same setup code on both if blocks, since that makes it
		harder to change stuff later on, but welp, that's all I can do ;-; */
		// no lol you just need to learn about class inheritance --raydeeux

		CCNode* indicator;
		if (Mod::get()->getSettingValue<bool>("show-as-image")) {
			indicator = CCSprite::create("cbf.png"_spr);
			static_cast<CCSprite*>(indicator)->setOpacity(50); // DANG IT WHY DIDN'T I THINK OF USING A STATIC CAST IN MY OLD CODE ;-;
			indicator->setScale(.2f);
			
		} else {
			indicator = CCLabelBMFont::create("CBF", "bigFont.fnt");
			static_cast<CCLabelBMFont*>(indicator)->setOpacity(50);
			indicator->setScale(.3f);
		}

		positionIndicator(indicator);
		indicator->setVisible(g_isCBFOn);

		indicator->setID("cbf_indicator"_spr);

		this->addChild(indicator);
		this->addEventListener<ToggleCBFEventFilter>([indicator](bool on) {
			indicator->setVisible(on);
			return ListenerResult::Stop;
		});

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
		if (!g_isCBFOn) return;

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