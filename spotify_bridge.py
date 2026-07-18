from flask import Flask
import spotipy
from spotipy.oauth2 import SpotifyOAuth

# 1. Create an app at https://developer.spotify.com/dashboard
# 2. Create the app first with just a name/description.
# 3. Once created, click "Edit Settings".
# 4. Add this exact Redirect URI: http://YOUR_IP_ADDRESS:8888/
#    (Replace YOUR_IP_ADDRESS with the actual IP of the machine running this script)
CLIENT_ID = '192.168.1.114'
CLIENT_SECRET = '43ef35a02eaf40d8913ee9ad73158cff'
REDIRECT_URI = 'http://192.168.1.114:8888/'

app = Flask(__name__)

# Scope required to read currently playing track
scope = "user-read-currently-playing"

# Setup OAuth
# Note: If you get a "redirect_uri_mismatch" error, ensure the URI in the 
# Spotify Dashboard matches the REDIRECT_URI variable above exactly.
sp_oauth = SpotifyOAuth(client_id=CLIENT_ID,
                        client_secret=CLIENT_SECRET,
                        redirect_uri=REDIRECT_URI,
                        scope=scope)

@app.route('/spotify')
def get_spotify_status():
    # Get cached token or prompt for login if needed
    token_info = sp_oauth.get_cached_token()
    
    if not token_info:
        return "Not authenticated. Please run the script locally first to authorize.", 401

    sp = spotipy.Spotify(auth=token_info['access_token'])
    
    try:
        current_track = sp.current_user_playing_track()
        
        if current_track and current_track['is_playing']:
            track_name = current_track['item']['name']
            artist_name = current_track['item']['artists'][0]['name']
            # Return the format the ESP32 expects
            return f"{track_name} - {artist_name}"
        else:
            return "Paused - Spotify"
    except Exception as e:
        return f"Error: {str(e)}", 500

if __name__ == '__main__':
    # Run on port 5000. 
    # Make sure your ESP32 code points to http://<YOUR_PC_IP>:5000/spotify
    app.run(host='0.0.0.0', port=5000)
