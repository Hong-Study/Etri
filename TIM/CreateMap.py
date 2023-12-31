from staticmap import StaticMap, CircleMarker

def CreatePendingMap(center_lat, center_long, lat, long, zoom_level=15):
    m = StaticMap(400, 400, tile_request_timeout=1.0)
    marker = CircleMarker((long, lat), '#0036FF', 5)
    m.add_marker(marker)
    image = m.render(zoom=zoom_level, center=[center_long, center_lat])
    image.save('SaveMap/map.png')

def CreatePairingMap(center_lat, center_long, tifd_lat, tifd_long, tird_lat, tird_long, zoom_level=15):
    m = StaticMap(400, 400, tile_request_timeout=1.0)
    tifdMarker = CircleMarker((tifd_long, tifd_lat), '#0036FF', 5)
    tirdMarker = CircleMarker((tird_long, tird_lat), '#FF0000', 5)
    m.add_marker(tifdMarker)
    m.add_marker(tirdMarker)

    image = m.render(zoom=zoom_level, center=[center_long, center_lat])
    image.save('SaveMap/map.png')