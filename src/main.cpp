#include <Geode/Geode.hpp>
#include <Geode/Loader.hpp>

#include <Geode/modify/UILayer.hpp>

using namespace geode::prelude;

class $modify(CBFIndUILayer, UILayer) {
	bool init(GJBaseGameLayer* layer) {
		if (!UILayer::init(layer)) return false;

		auto size = cocos2d::CCDirector::sharedDirector()->getWinSize();

		auto indicator = CCSprite::create("cbf.png"_spr);
		indicator->setScale(.5f);
		indicator->setPosition({ 0, size.height });
		indicator->setAnchorPoint({ 0.f, 1.f });
		indicator->setOpacity(30);
		indicator->setScale(.5f);

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