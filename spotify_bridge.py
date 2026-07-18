from flask import Flask
import spotipy
from spotipy.oauth2 import SpotifyOAuth

# 1. Create an app at https://developer.spotify.com/dashboard
# 2. Set your Redirect URI to http://localhost:8888/callback
CLIENT_ID = 'YOUR_CLIENT_ID'
CLIENT_SECRET = 'YOUR_CLIENT_SECRET'
REDIRECT_URI = 'http://localhost:8888/callback'

app = Flask(__name__)

# Scope required to read currently playing track
scope = "user-read-currently-playing"

# Setup OAuth
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
