#include <doctest/doctest.h>

#include "MyRevoke/EventSystem/AppEvent.h"
#include "MyRevoke/EventSystem/KeyEvent.h"
#include "MyRevoke/EventSystem/MouseEvent.h"

using namespace Revoke;

TEST_CASE("Every event reports its own type and name")
{
	CHECK(WindowResizeEvent(1280, 720).GetEventType() == EventType::WindowResize);
	CHECK(std::string(WindowResizeEvent(1280, 720).GetName()) == "WindowResize");

	CHECK(WindowsCloseEvent().GetEventType() == EventType::WindowClose);
	CHECK(std::string(WindowsCloseEvent().GetName()) == "WindowClose");

	CHECK(KeyPressedEvent(65, 0).GetEventType() == EventType::KeyPressed);
	CHECK(std::string(KeyPressedEvent(65, 0).GetName()) == "KeyPressed");

	CHECK(KeyReleasedEvent(65).GetEventType() == EventType::KeyReleased);
	CHECK(std::string(KeyReleasedEvent(65).GetName()) == "KeyReleased");

	CHECK(KeyTypedEvent(65).GetEventType() == EventType::KeyTyped);
	CHECK(std::string(KeyTypedEvent(65).GetName()) == "KeyTyped");

	CHECK(MouseMovedEvent(0.0f, 0.0f).GetEventType() == EventType::MouseMoved);
	CHECK(std::string(MouseMovedEvent(0.0f, 0.0f).GetName()) == "MouseMoved");

	CHECK(MouseButtonPressedEvent(0).GetEventType() == EventType::MouseButtonPressed);
	CHECK(std::string(MouseButtonPressedEvent(0).GetName()) == "MouseButtonPressed");

	CHECK(MouseButtonReleasedEvent(0).GetEventType() == EventType::MouseButtonReleased);
	CHECK(std::string(MouseButtonReleasedEvent(0).GetName()) == "MouseButtonReleased");

	CHECK(MouseScrolledEvent(0.0, 0.0).GetEventType() == EventType::MouseScrolled);
	CHECK(std::string(MouseScrolledEvent(0.0, 0.0).GetName()) == "MouseScrolled");
}

TEST_CASE("Static and virtual event types agree")
{
	CHECK(KeyPressedEvent(65, 0).GetEventType() == KeyPressedEvent::GetStaticType());
	CHECK(MouseMovedEvent(0.0f, 0.0f).GetEventType() == MouseMovedEvent::GetStaticType());
	CHECK(WindowResizeEvent(1280, 720).GetEventType() == WindowResizeEvent::GetStaticType());
}

TEST_CASE("Window events are in the app category only")
{
	WindowResizeEvent resize(1280, 720);

	CHECK(resize.IsCategory(EventCategoryApp));
	CHECK_FALSE(resize.IsCategory(EventCategoryInput));
	CHECK_FALSE(resize.IsCategory(EventCategoryKeyboard));
	CHECK_FALSE(resize.IsCategory(EventCategoryMouse));
	CHECK_FALSE(resize.IsCategory(EventCategoryMouseButton));

	WindowsCloseEvent close;

	CHECK(close.IsCategory(EventCategoryApp));
	CHECK_FALSE(close.IsCategory(EventCategoryInput));
}

TEST_CASE("Key events are in the keyboard and input categories")
{
	KeyPressedEvent pressed(65, 0);

	CHECK(pressed.IsCategory(EventCategoryKeyboard));
	CHECK(pressed.IsCategory(EventCategoryInput));
	CHECK_FALSE(pressed.IsCategory(EventCategoryApp));
	CHECK_FALSE(pressed.IsCategory(EventCategoryMouse));

	KeyReleasedEvent released(65);

	CHECK(released.IsCategory(EventCategoryKeyboard));
	CHECK(released.IsCategory(EventCategoryInput));

	KeyTypedEvent typed(65);

	CHECK(typed.IsCategory(EventCategoryKeyboard));
	CHECK(typed.IsCategory(EventCategoryInput));
}

TEST_CASE("Mouse events are in the mouse and input categories")
{
	MouseMovedEvent moved(0.0f, 0.0f);

	CHECK(moved.IsCategory(EventCategoryMouse));
	CHECK(moved.IsCategory(EventCategoryInput));
	CHECK_FALSE(moved.IsCategory(EventCategoryKeyboard));

	MouseScrolledEvent scrolled(0.0, 0.0);

	CHECK(scrolled.IsCategory(EventCategoryMouse));
	CHECK(scrolled.IsCategory(EventCategoryInput));

	MouseButtonPressedEvent buttonPressed(0);

	CHECK(buttonPressed.IsCategory(EventCategoryMouse));
	CHECK(buttonPressed.IsCategory(EventCategoryInput));

	MouseButtonReleasedEvent buttonReleased(0);

	CHECK(buttonReleased.IsCategory(EventCategoryMouse));
	CHECK(buttonReleased.IsCategory(EventCategoryInput));
}

TEST_CASE("Key events keep the data they were constructed with")
{
	KeyPressedEvent pressed(65, 3);

	CHECK(pressed.GetKeyCode() == 65);
	CHECK(pressed.GetRepeatCount() == 3);

	KeyReleasedEvent released(66);

	CHECK(released.GetKeyCode() == 66);

	KeyTypedEvent typed(67);

	CHECK(typed.GetKeyCode() == 67);
}

TEST_CASE("Mouse events keep the data they were constructed with")
{
	MouseMovedEvent moved(12.5f, -3.25f);

	CHECK(moved.GetX() == doctest::Approx(12.5f));
	CHECK(moved.GetY() == doctest::Approx(-3.25f));

	MouseScrolledEvent scrolled(1.5, -0.75);

	CHECK(scrolled.GetDeltaX() == doctest::Approx(1.5));
	CHECK(scrolled.GetDeltaY() == doctest::Approx(-0.75));

	MouseButtonPressedEvent buttonPressed(1);

	CHECK(buttonPressed.GetMouseButton() == 1);

	MouseButtonReleasedEvent buttonReleased(2);

	CHECK(buttonReleased.GetMouseButton() == 2);
}

TEST_CASE("Window resize keeps the size it was constructed with")
{
	WindowResizeEvent resize(1280, 720);

	CHECK(resize.GetWidth() == 1280u);
	CHECK(resize.GetHeight() == 720u);
}

TEST_CASE("Events start out unhandled")
{
	KeyPressedEvent pressed(65, 0);

	CHECK_FALSE(pressed.IsHandeled());
	CHECK_FALSE(pressed.Handled);
}

TEST_CASE("Dispatcher only calls the handler whose type matches the event")
{
	KeyPressedEvent pressed(65, 0);
	EventDispatcher dispatcher(pressed);

	int releasedCalls = 0;
	int pressedCalls = 0;

	bool releasedDispatched = dispatcher.Dispatch<KeyReleasedEvent>([&](KeyReleasedEvent&)
		{
			releasedCalls++;
			return true;
		});
	bool pressedDispatched = dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent&)
		{
			pressedCalls++;
			return true;
		});

	CHECK_FALSE(releasedDispatched);
	CHECK(releasedCalls == 0);

	CHECK(pressedDispatched);
	CHECK(pressedCalls == 1);
}

TEST_CASE("Dispatcher hands the handler the original event, with its data intact")
{
	MouseScrolledEvent scrolled(2.0, -1.0);
	EventDispatcher dispatcher(scrolled);

	MouseScrolledEvent* handlerArgument = nullptr;

	dispatcher.Dispatch<MouseScrolledEvent>([&](MouseScrolledEvent& event)
		{
			handlerArgument = &event;
			return true;
		});

	CHECK(handlerArgument == &scrolled);
	CHECK(handlerArgument->GetDeltaX() == doctest::Approx(2.0));
	CHECK(handlerArgument->GetDeltaY() == doctest::Approx(-1.0));
}

TEST_CASE("Handled follows what the matching handler returned")
{
	SUBCASE("handler returning true marks the event handled")
	{
		WindowResizeEvent resize(1280, 720);
		EventDispatcher dispatcher(resize);

		dispatcher.Dispatch<WindowResizeEvent>([](WindowResizeEvent&) { return true; });

		CHECK(resize.Handled);
		CHECK(resize.IsHandeled());
	}

	SUBCASE("handler returning false leaves the event unhandled")
	{
		WindowResizeEvent resize(1280, 720);
		EventDispatcher dispatcher(resize);

		dispatcher.Dispatch<WindowResizeEvent>([](WindowResizeEvent&) { return false; });

		CHECK_FALSE(resize.Handled);
		CHECK_FALSE(resize.IsHandeled());
	}

	SUBCASE("a handler for another type leaves the event unhandled")
	{
		WindowResizeEvent resize(1280, 720);
		EventDispatcher dispatcher(resize);

		dispatcher.Dispatch<WindowsCloseEvent>([](WindowsCloseEvent&) { return true; });

		CHECK_FALSE(resize.Handled);
		CHECK_FALSE(resize.IsHandeled());
	}
}

TEST_CASE("Dispatching the same event twice runs both matching handlers")
{
	MouseButtonPressedEvent buttonPressed(1);
	EventDispatcher dispatcher(buttonPressed);

	int calls = 0;

	dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent&)
		{
			calls++;
			return false;
		});
	dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent&)
		{
			calls++;
			return true;
		});

	CHECK(calls == 2);
	CHECK(buttonPressed.Handled);
}
