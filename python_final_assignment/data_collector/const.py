API_KEY = 'a3e0ae07-e656-44a4-9691-cfa43dacf086'
URLS_PREFIX = 'https://api.um.warszawa.pl/api/action/'

BUSES_LOC_RESOURCE_ID = 'f2e5503e-927d-4ad3-9500-4ab9e55deb59'
BUSES_LOC_URL = URLS_PREFIX + 'busestrams_get/'

VEHICLE_TYPE = '1'
BUSES_LOC_REQUEST_PARAMS = {
    'resource_id': BUSES_LOC_RESOURCE_ID,
    'apikey': API_KEY,
    'type': VEHICLE_TYPE
}

BUSES_UPDATE_PERIOD = 10

STOPS_COORDS_RESOURCE_ID = 'ab75c33d-3a26-4342-b36a-6e5fef0a3ac3'

# Define the base URL and parameters separately
STOPS_COORDS_BASE_URL = f"{URLS_PREFIX}dbstore_get"
STOPS_COORDS_PARAMS = f"id={STOPS_COORDS_RESOURCE_ID}&apikey={API_KEY}"
# Construct the full URL by combining them
STOPS_COORDS_URL = f"{STOPS_COORDS_BASE_URL}?{STOPS_COORDS_PARAMS}"

SCHEDULES_ENDPOINT = URLS_PREFIX + "dbtimetable_get"

LINES_PATHS_URL = URLS_PREFIX + 'public_transport_routes/'

LINES_PATHS_PARAMS = {
    'apikey': API_KEY
}
