#!/usr/bin/env python

import rpcz

from gen_proto import geometry_msgs_pb2
from gen_proto import graspable_object_pb2
from gen_proto import run_recognition_pb2
from gen_proto import get_camera_origin_pb2
from gen_proto import check_grasp_reachability_pb2

from gen_proto import run_recognition_rpcz
from gen_proto import get_camera_origin_rpcz
from gen_proto import check_grasp_reachability_rpcz



class RecognitionService(run_recognition_rpcz.ObjectRecognitionService):
	def __init__(self):
		self.count = 0

	def RunObjectRecognition(self,request,reply):
		print "received RunObjectRecognition request " + str(self.count)
		self.count += 1
		response = run_recognition_pb2.ObjectRecognitionResponse()
		reply.send(response)

class CameraOriginService(get_camera_origin_rpcz.CameraOriginService):
	def __init__(self):
		self.count = 0

	def GetCameraOrigin(self,request,reply):
		print "received GetCameraOrigin request " + str(self.count)
		self.count += 1
		response = get_camera_origin_pb2.CameraOriginResponse()
		reply.send(response)

class CheckGraspReachabilityService(check_grasp_reachability_rpcz.CheckGraspReachabilityService):
	def __init__(self):
		self.count = 0

	def CheckGraspReachability(self,request,reply):
		print "received CheckGraspReachability request "+ str(self.count)
		self.count += 1
		response = check_grasp_reachability_pb2.CheckGraspReachabilityResponse()
		reply.send(response)


app = rpcz.Application()
server = rpcz.Server(app)

server.register_service(RecognitionService(), "RecognitionService")
server.register_service(CameraOriginService(), "CameraOriginService")
server.register_service(CheckGraspReachabilityService(), "CheckGraspReachabilityService")

server.bind("tcp://*:5556")
print "Serving requests on port 5556"

app.run()
