/*
 *  WindowLinux.cpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include "Window.hpp"
#include <Engine/Log/Log.hpp>
#include <xcb/xcb.h>

using namespace Kompot;

struct Kompot::PlatformHandlers
{
    xcb_connection_t* xcbConnection;
    xcb_screen_t* xcbScreen;
    xcb_window_t xcbWindow;
};

xcb_intern_atom_reply_t* reply2;

Window::Window(std::string_view windowName, Kompot::IRenderer* renderer, const PlatformHandlers* parentWindowHandlers) :
    mWindowName(windowName), mRenderer(renderer), mParentWindowHandlers(parentWindowHandlers)
{
    Log& log                       = Log::getInstance();
    mWindowHandlers                = new PlatformHandlers{};
    mWindowHandlers->xcbConnection = xcb_connect(nullptr, nullptr);
    assert(mWindowHandlers->xcbConnection);
    auto setup              = xcb_get_setup(mWindowHandlers->xcbConnection);
    auto xcbScreenIteratior = xcb_setup_roots_iterator(setup);
    for (uint32_t i = 1; xcbScreenIteratior.rem; xcb_screen_next(&xcbScreenIteratior), ++i)
    {
        mWindowHandlers->xcbScreen = xcbScreenIteratior.data;
        log << "Founded screen #" << i << " (" << mWindowHandlers->xcbScreen->width_in_pixels << '*' << mWindowHandlers->xcbScreen->height_in_pixels
            << ")" << std::endl;
    }
    mWindowHandlers->xcbWindow = xcb_generate_id(mWindowHandlers->xcbConnection);

    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    std::array<uint32_t, 2> flagValues{
        XCB_NONE,
        XCB_EVENT_MASK_NO_EVENT | XCB_EVENT_MASK_NO_EVENT |

            // changed state or covering
            XCB_EVENT_MASK_EXPOSURE |

            // some key was presser while window in focus
            XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |

            // mouse button
            XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |

            // move mouse while x button is pressed
            XCB_EVENT_MASK_POINTER_MOTION | // no mouse button held
            XCB_EVENT_MASK_POINTER_MOTION_HINT |

            XCB_EVENT_MASK_BUTTON_MOTION | // one or more
            XCB_EVENT_MASK_BUTTON_1_MOTION | XCB_EVENT_MASK_BUTTON_2_MOTION | XCB_EVENT_MASK_BUTTON_3_MOTION | XCB_EVENT_MASK_BUTTON_4_MOTION |
            XCB_EVENT_MASK_BUTTON_5_MOTION |

            //  mouse poineter enters/leaves the window
            XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |

            // other

            XCB_EVENT_MASK_KEYMAP_STATE | XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_VISIBILITY_CHANGE | XCB_EVENT_MASK_RESIZE_REDIRECT |
            XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_PROPERTY_CHANGE |
            XCB_EVENT_MASK_COLOR_MAP_CHANGE | XCB_EVENT_MASK_OWNER_GRAB_BUTTON};
    xcb_create_window(
        mWindowHandlers->xcbConnection,
        XCB_COPY_FROM_PARENT,
        mWindowHandlers->xcbWindow,
        mWindowHandlers->xcbScreen->root,
        0,
        0,
        500, // engineConfig.windowWidth,
        500, // engineConfig.windowHeight,
        10,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        mWindowHandlers->xcbScreen->root_visual,
        mask,
        flagValues.data());

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(mWindowHandlers->xcbConnection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* reply  = xcb_intern_atom_reply(mWindowHandlers->xcbConnection, cookie, 0);

    xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(mWindowHandlers->xcbConnection, 0, 16, "WM_DELETE_WINDOW");
    reply2                           = xcb_intern_atom_reply(mWindowHandlers->xcbConnection, cookie2, 0);

    xcb_change_property(mWindowHandlers->xcbConnection, XCB_PROP_MODE_REPLACE, mWindowHandlers->xcbWindow, (*reply).atom, 4, 32, 1, &(*reply2).atom);

    xcb_map_window(mWindowHandlers->xcbConnection, mWindowHandlers->xcbWindow);
    xcb_flush(mWindowHandlers->xcbConnection);

    mWindowRendererAttributes = renderer->updateWindowAttributes(this);
}

Window::~Window()
{
    xcb_disconnect(mWindowHandlers->xcbConnection);
}

void Window::run()
{
    Log& log                      = Log::getInstance();
    bool needToExit               = false;
    xcb_generic_event_t* xcbEvent = xcb_wait_for_event(mWindowHandlers->xcbConnection);
    while (!needToExit)
    {
        xcbEvent = xcb_poll_for_event(mWindowHandlers->xcbConnection);
        if (xcbEvent == nullptr)
        {
            // mNeedToExit = true;
            continue;
        }

        switch (xcbEvent->response_type & ~0x80)
        {
        case XCB_KEY_PRESS:
            /* Handle the ButtonPress event type */
            // xcb_button_press_event_t* xcbButtonPressEvent =
            // reinterpret_cast<xcb_button_press_event_t*>(xcbEvent); log << xcbButtonPressEvent->
            // << std::endl;
            break;
        case XCB_KEY_RELEASE:
            // xcb_key_release_event_t
            break;
        case XCB_BUTTON_PRESS:
            // xcb_button_press_event_t
            break;
        case XCB_BUTTON_RELEASE:
            //  xcb_button_release_event_t
            break;
        case XCB_MOTION_NOTIFY:
            // xcb_motion_notify_event_t
            break;
        case XCB_ENTER_NOTIFY:
            break;
        case XCB_EXPOSE:
        {
            xcb_expose_event_t* xcbExposeEvent = reinterpret_cast<xcb_expose_event_t*>(xcbEvent);

            log << "Window " << xcbExposeEvent->window << " exposed. Region to be redrawn at location (" << xcbExposeEvent->x << ","
                << xcbExposeEvent->y << "), with dimension (" << xcbExposeEvent->width << "," << xcbExposeEvent->height << ")\n"
                << std::endl;
            break;
        }
        case XCB_CLIENT_MESSAGE:
        {
            xcb_client_message_event_t* xcbClientMessageEvent = reinterpret_cast<xcb_client_message_event_t*>(xcbEvent);
            if (reply2->atom == xcbClientMessageEvent->data.data32[0])
            {
                needToExit = true;
            }
            xcbClientMessageEvent->data.data16[9] = 4;
            break;
        }
        default:
        {
            log << "Unknown XCB event received" << std::endl;
            break;
        }
        }
        free(xcbEvent);
    }
    // conditionVariable.wait()
}
