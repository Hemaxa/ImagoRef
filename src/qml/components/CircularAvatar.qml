import QtQuick
import QtQuick.Effects

Item {
    id: root

    property url source: ""
    property string fallbackText: "?"
    property color backgroundColor: "#f3dcc7"
    property color borderColor: "#181818"
    property color textColor: "#141414"
    property int borderWidth: 1
    property int fallbackFontPixelSize: Math.round(Math.min(width, height) * 0.48)
    readonly property bool imageReady: source !== "" && avatarSource.status === Image.Ready

    Rectangle {
        anchors.fill: parent
        radius: Math.min(width, height) / 2
        color: root.backgroundColor
        border.width: root.borderWidth
        border.color: root.borderColor
        antialiasing: true
    }

    Item {
        id: avatarInset
        anchors.fill: parent
        anchors.margins: root.borderWidth
    }

    Image {
        id: avatarSource
        anchors.fill: avatarInset
        source: root.source
        fillMode: Image.PreserveAspectCrop
        cache: false
        asynchronous: true
        smooth: true
        opacity: 0
    }

    Rectangle {
        id: avatarMask
        anchors.fill: avatarInset
        radius: Math.min(width, height) / 2
        color: "black"
        opacity: 0
        antialiasing: true
        layer.enabled: true
    }

    MultiEffect {
        anchors.fill: avatarInset
        source: avatarSource
        maskEnabled: true
        maskSource: avatarMask
        maskThresholdMin: 0.5
        autoPaddingEnabled: false
        visible: root.imageReady
    }

    Text {
        anchors.centerIn: parent
        text: root.fallbackText
        font.pixelSize: root.fallbackFontPixelSize
        font.bold: true
        color: root.textColor
        visible: !root.imageReady
    }
}
