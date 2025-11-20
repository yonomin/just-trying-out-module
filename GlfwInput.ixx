module;
#include <GLFW/glfw3.h>

export module GlfwInput;
import <unordered_map>;
import GlfwUse;

struct InputState
{
	bool currentIsDown{ false };
	bool justPressed{ false };
	bool justReleased{ false };
};

//本类目前没有设置y轴offset的反转
export class GlfwInput
{
public:
	//鼠标
	//是否开启点击
	bool enableClickInput{};
	//鼠标指针 捕捉状态
	bool capturingCursor{};//开启捕捉
	bool firstFrameOfCapturingCursor{};//是捕捉指针的第一帧
	bool wasCapturingCursorLastFrame{};//上一帧正在捕捉指针
	//鼠标指针 移动
	bool cursorMovedInThisFrame{};//鼠标指针是否在本帧移动
	double xPosCurrentFrame{}, yPosCurrentFrame{};//本帧鼠标指针位置
	double xPosLastFrame{}, yPosLastFrame{};//上帧鼠标指针位置
	double xPosOffsetBetweenTwoFrames{}, yPosOffsetBetweenTwoFrames{};//本帧与上一帧之间的位移距离
	//鼠标滚轮
	double xOffsetScroll{}, yOffsetScroll{};//滚轮滚动距离 ?怎么计算的
private:
	GLFWwindow &window_ref;//处理该窗口的输入
	std::unordered_map<int, InputState> keyStates;//键盘状态
	std::unordered_map<int, InputState> MouseButtonStates;//鼠标按键状态

	//函数 == == == == == == == == == == == == == == 
public:
	//构造函数
	explicit GlfwInput(GLFWwindow &ptr_window);
	//更新输入 
	void update();

	//回调函数
	//鼠标指针位置回调函数
	static void cursorPosCallback(GLFWwindow *window_ptr, double xPos, double yOos);
	//鼠标滚轮回调函数
	static void scrollCallBack(GLFWwindow *window_ptr, double xOffset, double yOffset);
	//键盘按键回调函数
	static void keyCallback(GLFWwindow *window_ptr, int key, int scancode, int action, int mods);
	//鼠标按键回调函数
	static void mouseButtonCallback(GLFWwindow *window_ptr, int button, int action, int mods);
private:
	//鼠标
	//GLFW鼠标捕捉的开关
	void cursorCaptureSwitch();
	//清理上一帧的状态遗留
	void clearLastFrameStates();
	//更新按键状态的通用函数
	void updateKeysAndMouseButtomsState(int action, InputState &state);
};

module: private;
//实现 == == == == == == == == == == == == == == 
GlfwInput::GlfwInput(GLFWwindow &window_ptr) :
	window_ref(window_ptr)
{
	glfwSetCursorPosCallback(&window_ref, cursorPosCallback);
	glfwSetScrollCallback(&window_ref, scrollCallBack);
	glfwSetKeyCallback(&window_ref, keyCallback);
	glfwSetMouseButtonCallback(&window_ref, mouseButtonCallback);
}

void GlfwInput::update()
{
	clearLastFrameStates();
	cursorCaptureSwitch();
}
//指针位置回调函数
void GlfwInput::cursorPosCallback(GLFWwindow *ptr_window, double xpos, double ypos)
{
	//创造userpointer
	windowContext *context = static_cast<windowContext *>(glfwGetWindowUserPointer(ptr_window));
	if (context && context->input_context)
	{
		//获取引用 
		double &xPosLast = context->input_context->xPosLastFrame;
		double &yPosLast = context->input_context->yPosLastFrame;
		double &xPosCurrent = context->input_context->xPosCurrentFrame;
		double &yPosCurrent = context->input_context->yPosCurrentFrame;
		double &xPosOffset = context->input_context->xPosOffsetBetweenTwoFrames;
		double &yPosOffset = context->input_context->yPosOffsetBetweenTwoFrames;
		bool &moved = context->input_context->cursorMovedInThisFrame;
		bool &firstCapture = context->input_context->firstFrameOfCapturingCursor;

		//本帧移动设为true
		moved = true;

		//保存上一帧位置数据
		xPosLast = xPosCurrent;
		yPosLast = yPosCurrent;
		//更新当前帧位置数据
		xPosCurrent = xpos;
		yPosCurrent = ypos;

		//本帧为捕捉首帧
		if (firstCapture)
		{
			//offset归零防鼠标瞬移
			xPosOffset = 0.0f;
			yPosOffset = 0.0f;
			firstCapture = false;
		}
		//本帧非首帧
		else
		{
			//正常更新offset
			xPosOffset = xPosCurrent - xPosLast;
			yPosOffset = yPosCurrent - yPosLast;
		}
	}
}
//滚轮回调函数
void GlfwInput::scrollCallBack(GLFWwindow *ptr_window, double xOffset, double yOffset)
{
	windowContext *context = static_cast<windowContext *>(glfwGetWindowUserPointer(ptr_window));
	if (context && context->input_context)
	{
		double &yOffsetScroll = context->input_context->yOffsetScroll;

		yOffsetScroll = yOffset;
	}
}

void GlfwInput::keyCallback(GLFWwindow *ptr_window, int key, int scancode, int action, int mods)
{
	windowContext *context = static_cast<windowContext *>(glfwGetWindowUserPointer(ptr_window));
	if (context && context->input_context)
	{
		//取出key按键的状态对象
		InputState &state = context->input_context->keyStates[key];
		context->input_context->updateKeysAndMouseButtomsState(action, state);
	}
}

void GlfwInput::mouseButtonCallback(GLFWwindow *ptr_window, int button, int action, int mods)
{
	windowContext *context = static_cast<windowContext *>(glfwGetWindowUserPointer(ptr_window));
	if (context && context->input_context)
	{
		//取出button按键的状态对象
		InputState &state = context->input_context->MouseButtonStates[button];
		context->input_context->updateKeysAndMouseButtomsState(action, state);
	}
}

// private 

void GlfwInput::cursorCaptureSwitch()
{
	//如果上帧的捕捉状态和本帧相同
	if (wasCapturingCursorLastFrame == capturingCursor) return;
	//如果需要显示指针
	if (!capturingCursor)
		//切换到显示指针模式
		glfwSetInputMode(&window_ref, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	//若不需要显示指针
	else
		//切换到锁定指针模式
		glfwSetInputMode(&window_ref, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//如果是刚开始捕捉
	if (capturingCursor && !wasCapturingCursorLastFrame)
	{
		//首次捕捉=true
		firstFrameOfCapturingCursor = true;
	}
	//保存上帧捕捉状态
	wasCapturingCursorLastFrame = capturingCursor;
}

void GlfwInput::clearLastFrameStates()
{
	//重置按键状态
	for (auto &pair : keyStates)
	{
		pair.second.justPressed = false;
		pair.second.justReleased = false;
	}
	for (auto &pair : MouseButtonStates)
	{
		pair.second.justPressed = false;
		pair.second.justReleased = false;
	}
	//重置鼠标指针状态
	cursorMovedInThisFrame = false;
	//清理滚轮增量
	yOffsetScroll = 0.0;
	//重置首次捕捉状态
	firstFrameOfCapturingCursor = false;
}

void GlfwInput::updateKeysAndMouseButtomsState(int action, InputState &state)
{
	if (action == GLFW_PRESS)
	{
		state.currentIsDown = true;
		state.justPressed = true;
		state.justReleased = false;
	}
	else if (action == GLFW_RELEASE)
	{
		state.currentIsDown = false;
		state.justPressed = false;
		state.justReleased = true;
	}
	else if (action == GLFW_REPEAT)
	{
		state.currentIsDown = true;
		state.justPressed = false;
		state.justReleased = false;
	}
}