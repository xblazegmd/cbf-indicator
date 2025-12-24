#include <Geode/Geode.hpp>
#include <Geode/Loader.hpp>
#include <Geode/loader/Mod.hpp>

#include <Geode/modify/UILayer.hpp>
#include <Geode/modify/EndLevelLayer.hpp>

using namespace geode::prelude;

class $modify(CBFIndUILayer, UILayer) {
	bool init(GJBaseGameLayer* layer) {
		if (!UILayer::init(layer)) return false;

		auto size = cocos2d::CCDirector::sharedDirector()->getWinSize();

		auto indicator = CCSprite::create("cbf.png"_spr);

		indicator->setScale(.2f);
		indicator->setPosition({ 0, size.height });
		indicator->setAnchorPoint({ 0.f, 1.f });
		indicator->setOpacity(50);

		indicator->setID("cbf_indicator"_spr);

		this->addChild(indicator);

		auto cbf = Loader::get()->getLoadedMod("syzzi.click_between_frames");
		bool isCBFOn = cbf && !cbf->getSettingValue<bool>("soft-toggle");
		indicator->setVisible(isCBFOn);

		listenForSettingChanges<bool>("soft-toggle", [indicator](bool val) {
			indicator->setVisible(!val);
		}, cbf);

		return true;
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
		logo->setPosition({ 402.f, 225.f });
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