/*
*   Copyright (C) 2019 by Maxim Stoyanov
*
*   scapior.github.io
*/

#include "ClientSubsystem.hpp"

using namespace KompotEngine;

ClientSubsystem::ClientSubsystem(int argc, char** argv, const EngineConfig& engineConfig)
{
    Log& log = Log::getInstance();
#ifdef ENGINE_PLATFORM_LINUX    
    static_cast<void>(argc);
    static_cast<void>(argv);
    m_xcbConnection = xcb_connect(nullptr, nullptr);
    assert(m_xcbConnection);
    auto setup = xcb_get_setup(m_xcbConnection);
    auto xcbScreenIteratior = xcb_setup_roots_iterator(setup);
    for (uint32_t i = 1; xcbScreenIteratior.rem; xcb_screen_next(&xcbScreenIteratior), ++i)
    {
        m_xcbScreen = xcbScreenIteratior.data;
        log << "Founded screen #" << i << " (" << m_xcbScreen->width_in_pixels << '*' << m_xcbScreen->height_in_pixels << ")" << std::endl;
    }
    m_xcbWindow = xcb_generate_id(m_xcbConnection);

    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    std::array<uint32_t, 2> flagValues{
                XCB_NONE,

                // changed state or covering
                XCB_EVENT_MASK_EXPOSURE |

                // some key was presser while window in focus
                XCB_EVENT_MASK_KEY_PRESS |
                XCB_EVENT_MASK_KEY_RELEASE |

                // mouse button
                XCB_EVENT_MASK_BUTTON_PRESS |
                XCB_EVENT_MASK_BUTTON_RELEASE |

                // move mouse while x button is pressed
                XCB_EVENT_MASK_POINTER_MOTION | // no mouse button held
                XCB_EVENT_MASK_BUTTON_MOTION |  // one or more
                XCB_EVENT_MASK_BUTTON_1_MOTION |
                XCB_EVENT_MASK_BUTTON_2_MOTION |
                XCB_EVENT_MASK_BUTTON_3_MOTION |
                XCB_EVENT_MASK_BUTTON_4_MOTION |
                XCB_EVENT_MASK_BUTTON_5_MOTION |

                //  mouse poineter enters/leaves the window
                XCB_EVENT_MASK_ENTER_WINDOW |
                XCB_EVENT_MASK_LEAVE_WINDOW |

                // other

                XCB_EVENT_MASK_VISIBILITY_CHANGE |
                XCB_EVENT_MASK_RESIZE_REDIRECT |
                XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
                XCB_EVENT_MASK_FOCUS_CHANGE |
                XCB_EVENT_MASK_OWNER_GRAB_BUTTON
    };
    xcb_create_window(
                m_xcbConnection,
                XCB_COPY_FROM_PARENT,
                m_xcbWindow,
                m_xcbScreen->root,
                0, 0,
                engineConfig.windowWidth,
                engineConfig.windowHeight,
                10,
                XCB_WINDOW_CLASS_INPUT_OUTPUT,
                m_xcbScreen->root_visual,
                mask,
                flagValues.data());
    xcb_map_window(m_xcbConnection, m_xcbWindow);
    xcb_flush(m_xcbConnection);
#endif
}

ClientSubsystem::~ClientSubsystem()
{
#ifdef ENGINE_PLATFORM_LINUX
    xcb_disconnect(m_xcbConnection);
#endif
}

void ClientSubsystem::run(std::condition_variable& conditionVariable)
{
    Log& log = Log::getInstance();
    xcb_generic_event_t *xcbEvent;
    while(!m_needToExit)
    {
        xcbEvent = xcb_poll_for_event(m_xcbConnection);
        if (xcbEvent == nullptr)
        {
            continue;
        }

        switch (xcbEvent->response_type & ~0x80)
        {
            case XCB_EXPOSE:
            {
                xcb_expose_event_t* xcbExposeEvent = reinterpret_cast<xcb_expose_event_t*>(xcbEvent);

                log << "Window " << xcbExposeEvent->window << " exposed. Region to be redrawn at location ("
                    << xcbExposeEvent->x << "," << xcbExposeEvent->y << "), with dimension ("
                    << xcbExposeEvent->width << "," << xcbExposeEvent->height << ")\n" << std::endl;
                break;
            }
            case XCB_BUTTON_PRESS:
            {
                /* Handle the ButtonPress event type */
                //xcb_button_press_event_t* xcbButtonPressEvent = reinterpret_cast<xcb_button_press_event_t*>(xcbEvent);
                //log << xcbButtonPressEvent-> << std::endl;


                /* ... */

                break;
            }
            default:
            {
                log << "Unknown XCB event received" << std::endl;
                break;
            }
        }
        /* Free the Generic Event */
        free (xcbEvent);
    }
    conditionVariable.wait()
}
