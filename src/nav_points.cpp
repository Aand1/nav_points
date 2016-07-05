#include <tf/transform_datatypes.h>
#include "nav_points/nav_points.h"
#include <XmlRpcException.h>

namespace nav_points {

NavPoints::NavPoints() : server_("nav_points") {
  ros::NodeHandle private_nh("~");

  std::string goal_topic;

  // this is where we will publish our goals to
  private_nh.param<std::string>("goal_topic", goal_topic,
                                "/move_base_simple/goal");

  goal_pub_ =
      private_nh.advertise<geometry_msgs::PoseStamped>(goal_topic, 10, false);

  // iterate through all points loaded
  // on parameter server and add markers
  private_nh.getParam("nav_points", param_points_);

  ROS_ASSERT(param_points_.getType() == XmlRpc::XmlRpcValue::TypeArray);

  ROS_INFO("size %d", param_points_.size());

  loadPoints();

  addMarkers();

  ros::spin();
}

void NavPoints::onClick(const visualization_msgs::InteractiveMarkerFeedbackConstPtr &f)
{
  if (f->event_type == visualization_msgs::InteractiveMarkerFeedback::MOUSE_UP){
    goal_pub_.publish(f->pose);
  }
}

void NavPoints::addMarkers()
{
  for (geometry_msgs::PoseStamped pose: points_)
  {
    server_.insert(createMarker(pose), boost::bind(&NavPoints::onClick, this, _1));
  }
}

visualization_msgs::InteractiveMarker NavPoints::createMarker(geometry_msgs::PoseStamped pose)
{
  visualization_msgs::InteractiveMarker int_marker;
  int_marker.header.frame_id = pose.header.frame_id;
  int_marker.scale = 1;
  int_marker.name = "map";

  visualization_msgs::InteractiveMarkerControl control;
  control.interaction_mode = visualization_msgs::InteractiveMarkerControl::BUTTON;
  control.name = "button";

  visualization_msgs::Marker shape;
  shape.type = visualization_msgs::Marker::ARROW;
  shape.scale.x = 0.1;
  shape.scale.z = 0.001;
  shape.scale.y = 0.02;
  shape.color.r=0;
  shape.color.g=0.5;
  shape.color.b=0.25;
  shape.color.a=1;

  control.markers.push_back(shape);
  control.always_visible = true;
  int_marker.controls.push_back(control);

  return int_marker;
}


void NavPoints::loadPoints() {
  for (int i = 0; i < param_points_.size(); i++) {
    if (param_points_[i].hasMember("x")) {
      double x = 0, y = 0, theta = 0;

      if (param_points_[i]["x"].getType() == XmlRpc::XmlRpcValue::TypeInt) {
        x = static_cast<int>(param_points_[i]["x"]);
      } else if (param_points_[i]["x"].getType() ==
                 XmlRpc::XmlRpcValue::TypeDouble) {
        x = static_cast<double>(param_points_[i]["x"]);
      } else
      {
        continue;
      }

      if (param_points_[i]["y"].getType() == XmlRpc::XmlRpcValue::TypeInt) {
        y = static_cast<int>(param_points_[i]["y"]);
      } else if (param_points_[i]["y"].getType() ==
                 XmlRpc::XmlRpcValue::TypeDouble) {
        y = static_cast<double>(param_points_[i]["y"]);
      } else
      {
        continue;
      }


      if (param_points_[i]["theta"].getType() == XmlRpc::XmlRpcValue::TypeInt) {
        theta = static_cast<int>(param_points_[i]["theta"]);
      } else if (param_points_[i]["theta"].getType() ==
                 XmlRpc::XmlRpcValue::TypeDouble) {
        theta = static_cast<double>(param_points_[i]["theta"]);
      } else
      {
        continue;
      }

      ROS_INFO("adding point (%f,%f,%f)", x, y, theta);
      tf::Quaternion orientation = tf::createQuaternionFromYaw(theta);
      geometry_msgs::PoseStamped pose;
      pose.header.stamp = ros::Time::now();
      pose.header.frame_id = "map";
      pose.pose.position.x = x;
      pose.pose.position.y = y;
      pose.pose.orientation.x = orientation.getX();
      pose.pose.orientation.y = orientation.getY();
      pose.pose.orientation.z = orientation.getZ();
      pose.pose.orientation.w = orientation.getW();

      points_.push_back(pose);
    } else {
      ROS_ERROR("Elements of nav_points list must be of the format '{x: 0, y: "
                "0, theta: 0}'");
    }
  }
}
}

int main(int argc, char **argv) {
  ros::init(argc, argv, "nav_points");
  nav_points::NavPoints pts;
}
