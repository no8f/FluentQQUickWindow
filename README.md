# FluentQQuickWindow

A very slimmed down QQuickWindow with support for the Mica material, it has no frame, i.e. you have full freedom in the design of the title bar.

My Implementaion may be shit :)

## Features
- **Backdrop suppport**: currently Mica, will add more
- **Global Dragging**: when enabled you can grab the window anywehre to drag it
- **Customisable Resize Margins**: you can change the width of the hit area for resize activation
- **Basic Window Funktionality**: min, max and floating state and resizing

## What Does Not Work
- ### Windows
  - **Animations**
  - **Aero snap** ⚠️ This could make the Window unusable for some applications
- ### Mac Os
  -⚠️Not Tested
- ## Linux
  -⚠️Not Tested; will follow sometime

## Usage

This thing does not compile into a lib, because its only two files.

- Download this repo
- include the the .cpp and .h in your project ( copy them or what ever ).
- link the dwmapi lib

  ```cmake
  target_link_libraries(<Your_Target>
    PRIVATE
    Qt6::Quick
    Qt6::Widgets
    dwmapi
  )
  ```
- include the .h in your main.cpp and call

  ```cpp
  CustomQQuickWindow::registerQmlType();
  ```
  before loading your main qml
- import the component in QML and use it as your app window, don't forget to set it transparent

  ```qml
  import FluentQQUickWindow 1.0

  CustomQQuickWindow {
    id: mainWindow
    width: 640
    height: 480
    visible: true
    title: "Test"
    color: "transparent"
    ...
  ```

> [!TIP]
> You can set your own URI by defining CUSTOMQML_URI
> ```cpp
>  #define CUSTOMQML_URI "customURI"
> ```

- The window remembers its own geometry opon closing, but an organisation name must be assigned for this

  ```cpp
    app.setOrganizationName("some-name");
  ```

## FAQ
---

F: Everything in the QML file is marked as a warning!

A: When all URI's are set correct, then this is a error of the QML Language Server. [Qt Blog](https://www.qt.io/blog/issues-with-qml-language-server-integration-in-qt-creator-14.0-and-15.0)

---

F: Is this production ready?

A: Fuck no

---

F: I did a much better job implementing stuff!

A: Concrats, make a PR.

---

F: Why would you build a shitty version of the original window

A: because Qt is to slow at implementing nice features and i like backdrop effects

--