#!/usr/bin/env python

import random
import tornado.ioloop
import tornado.web
import jinja2

random.seed()
env = jinja2.Environment()

class MainHandler( tornado.web.RequestHandler ):
    def get( self ):
        template = env.from_string('''
          <!DOCTYPE html>
          <html>
            <head>
            </head>
            <body>
              <div id="term">Here is where the server sent data will appear</div>
              <script type="text/javascript">
                if ( typeof( EventSource ) !== "undefined" ) {
                  var eSource = new EventSource( "pull.js" );
                  eSource.onmessage = function( event ) {
                    document.getElementById( "term" ).innerHTML = event.data;
                  };
                } else {
                  document.getElementById( "term" ).innerHTML = "Whoops!";
                }
              </script>
            </body>
          </html>''' )

        self.write( template.render() )

count = 0
class PullHandler( tornado.web.RequestHandler ):
    def initialize( self ):
        self.set_header('Content-Type', 'text/event-stream')
        self.set_header('Cache-Control', 'no-cache')

    def get( self ):
        global count
        if count < 3:
            self.write( u"data: New random number: %d\n\n" % random.randint( 0, 1000 ) )
            count += 1
        else:
            self.set_status( 404 )
        self.flush()

application = tornado.web.Application( [
    ( r"/", MainHandler ),
    ( r"/pull.js", PullHandler )
] )

if __name__ == "__main__":
    application.listen( 8888 )
    tornado.ioloop.IOLoop.instance().start()

