from flask import Flask, request, redirect
import spotipy
from spotipy.oauth2 import SpotifyOAuth

# 1. Create an app at https://developer.spotify.com/dashboard
# 2. Create the app first with just a name/description.
# 3. Once created, click "Edit Settings".
# 4. Add this exact Redirect URI: http://192.168.1.114:8888/callback
CLIENT_ID = 'da2963603f0847ef910b259537463c97'
CLIENT_SECRET = '43ef35a02eaf40d8913ee9ad73158cff'
REDIRECT_URI = 'http://192.168.1.114:8888/callback'

app = Flask(__name__)

# Scope required to read currently playing track
scope = "user-read-currently-playing"

# Setup OAuth
sp_oauth = SpotifyOAuth(client_id=CLIENT_ID,
                        client_secret=CLIENT_SECRET,
                        redirect_uri=REDIRECT_URI,
                        scope=scope,
                        open_browser=True)

@app.route('/')
def index():
    # If no code, provide a link to start the auth flow
    auth_url = sp_oauth.get_authorize_url()
    return f'<h1>Spotify Auth</h1><a href="{auth_url}">Click here to authenticate with Spotify</a>'

@app.route('/callback')
def callback():
    # This handles the redirect from Spotify
    code = request.args.get('code')
    if code:
        sp_oauth.get_access_token(code)
        return "Authentication successful! You can close this window."
    return "Authentication failed or cancelled."

@app.route('/spotify')
def get_spotify_status():
    print("Request received from ESP32!")
    # Get cached token
    token_info = sp_oauth.get_cached_token()
    
    if not token_info:
        return "Not authenticated.", 401

    sp = spotipy.Spotify(auth=token_info['access_token'])
    
    try:
        current_track = sp.current_user_playing_track()
        
        if current_track and current_track['is_playing']:
            track_name = current_track['item']['name']
            artist_name = current_track['item']['artists'][0]['name']
            return f"{track_name} - {artist_name}"
        else:
            return "Paused - Spotify"
    except Exception as e:
        return f"Error: {str(e)}", 500

if __name__ == '__main__':
    # Listen on port 8888 to handle the Spotify redirect
    app.run(host='0.0.0.0', port=8888)
