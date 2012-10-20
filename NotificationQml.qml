// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import QtQuick 1.0

Rectangle {
    width: 400
    height: 76
    color: "#c8000000"
    radius: 13
    z: -1
    scale: 1
    opacity: 1
    rotation: 0
    clip: true
    smooth: true
    visible: true

    Image {
        id: image1
        x: 8
        y: 13
        width: 46
        height: 50
        sourceSize.width: 159
        fillMode: Image.PreserveAspectFit
        source: "icons/icon_partners-325.png"
    }

    Text {
        id: titleId
        x: 81
        y: 13
        width: 24
        height: 15
        color: "#ffffff"
        text: qsTr("Title")
        font.bold: true
        styleColor: "#ffffff"
        font.pixelSize: 15
        objectName: "title"
    }

    Text {
        id: descriptionId
        x: 81
        y: 30
        color: "#ffffff"
        text: qsTr("Description")
        font.strikeout: false
        font.underline: false
        font.italic: false
        font.bold: false
        font.pixelSize: 14
        objectName: "description"
    }
}
