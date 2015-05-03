import json 

FORWARD = 1

class ControlParameters():
    def __init__(self):
        self.desiredSpeed = 0
        self.motorPwm = 0
        self.direction = FORWARD
        self.maxPosition = 0
        self.minPosition = 0

    def get_as_json(self):
        return json.dumps(self.__dict__)

if __name__ == '__main__':
    controlParameters =ControlParameters()
    print controlParameters.get_as_json()