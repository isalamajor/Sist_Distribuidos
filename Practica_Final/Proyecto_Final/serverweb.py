import time, logging
from wsgiref.simple_server import make_server
from spyne import Application, ServiceBase, Integer, Unicode, rpc
from spyne.protocol.soap import Soap11
from spyne.server.wsgi import WsgiApplication
from datetime import datetime


class FechaHora(ServiceBase):
    @rpc(_returns=Unicode)
    def fecha_hora(ctx):
        fecha_hora_actual = datetime.now()
        return fecha_hora_actual.strftime("%d/%m/%Y %H:%M:%S")

application = Application(services=[FechaHora], tns='http://tests.python-zeep.org/',
    in_protocol=Soap11(validator='lxml'), out_protocol=Soap11())
application = WsgiApplication(application)

if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    logging.getLogger('spyne.protocol.xml').setLevel(logging.DEBUG)
    logging.info("listening to http://127.0.0.1:8000; wsdl is at: http://localhost:8000/?wsdl ")
    server = make_server('127.0.0.1', 8000, application)
    server.serve_forever()

'''
python3 serverweb.py &
python3 -mzeep http://localhost:8000/?wsdl
'''