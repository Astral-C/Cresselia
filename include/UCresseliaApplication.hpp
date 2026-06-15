#pragma once

#include "UApplication.hpp"

class UCresseliaApplication : public UApplication {
	struct GLFWwindow* mWindow;
	class UCresseliaContext* mContext;

	virtual bool Execute(float deltaTime) override;

	
public:

	UCresseliaApplication();
	virtual ~UCresseliaApplication() {}

	virtual bool Setup() override;
	virtual bool Teardown() override;
};
