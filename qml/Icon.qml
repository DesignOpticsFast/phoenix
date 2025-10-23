import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    
    property string name: "save"
    property string style: "SharpSolid"
    property int size: 24
    property bool autoTheme: true
    property bool dark: Qt.application.palette.window.lightness < Qt.application.palette.windowText.lightness
    property int v: 0 // Version bump triggers reloads
    
    Image {
        id: iconImage
        anchors.fill: parent
        source: `image://phxicon/${root.name}?style=${root.style}&size=${root.size}&dark=${root.autoTheme ? (root.dark ? 1 : 0) : 0}&v=${root.v}`
        smooth: true
        fillMode: Image.PreserveAspectFit
        
        Accessible.name: root.name
        Accessible.description: `Icon: ${root.name}`
    }
    
    Connections {
        target: Qt.application
        function onPaletteChanged() {
            if (root.autoTheme) {
                root.v++;
            }
        }
        function onDevicePixelRatioChanged() {
            root.v++;
        }
    }
}