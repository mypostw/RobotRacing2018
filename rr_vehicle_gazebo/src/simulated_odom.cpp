/**
 * @brief 
 * 
 * @file simulated_odom.cpp
 * @author Toni Ogunmade(oluwatoni)
 * @date 2018-05-04
 */

#include "simulated_odom.hpp"

/**
 * @brief Construct a new Simulated Odom:: Simulated Odom object
 * 
 */
SimulatedOdom::SimulatedOdom() {
  // the noise sd for the angular and linear velocities
  if(!nh_.getParam("simulated_odometry/angular_velocity_sd", angular_vel_sd_)){
    ROS_ERROR("angular velocity standard deviation was not specified");
    ros::shutdown();
  }
  if(!nh_.getParam("simulated_odometry/linear_velocity_sd", linear_vel_sd_)){
    ROS_ERROR("linear velocity standard deviation was not specified");
    ros::shutdown();
  }
  if(!nh_.getParam("simulated_odometry/right_rear_wheel/diameter", wheel_diameter_)){
    ROS_ERROR("wheel diameter was not specified");
    ros::shutdown();
  }
  if(!nh_.getParam("simulated_odometry/wheel_to_wheel_dist", wheel_to_wheel_dist_)){
    ROS_ERROR("the wheel to wheel distance was not specified");
    ros::shutdown();
  }

  // initialize the odometry message
  odom_.header.frame_id = "odom";
  odom_.child_frame_id = "base_link";
  odom_.pose.pose.position.x = 0;
  odom_.pose.pose.position.y = 0;
  odom_.pose.pose.position.z = 0;
  odom_.pose.pose.orientation.w = 1;
  odom_.pose.pose.orientation.x = 0;
  odom_.pose.pose.orientation.y = 0;
  odom_.pose.pose.orientation.z = 0;
  odom_.pose.covariance = {0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0};
  odom_.twist.twist.linear.x = 0;
  odom_.twist.twist.linear.y = 0;
  odom_.twist.twist.linear.z = 0;
  odom_.twist.twist.angular.x = 0;
  odom_.twist.twist.angular.y = 0;
  odom_.twist.twist.angular.z = 0;
  odom_.twist.covariance = {0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0};
  odom_pub_ = nh_.advertise<nav_msgs::Odometry>("odom", 1);

  // create the subscriber
  joint_sub_ = nh_.subscribe("/rr_vehicle/joint_states", 1, &SimulatedOdom::jointStateCb, this);
}

void SimulatedOdom::jointStateCb(const sensor_msgs::JointState::ConstPtr &msg) {
  // get the steering data
  ROS_INFO("Steering");
  ROS_INFO("%f, %f", msg->position[LEFT_STEERING], msg->position[RIGHT_STEERING]);
  double steering_angle = (msg->position[LEFT_STEERING] +
                           msg->position[RIGHT_STEERING]) / 2.0;
  ROS_INFO("%f radians", steering_angle);

  // get each wheels velocity
  double wheel_velocity = (msg->velocity[FRONT_LEFT_WHEEL] +
                           msg->velocity[FRONT_RIGHT_WHEEL] +
                           msg->velocity[REAR_LEFT_WHEEL] +
                           msg->velocity[REAR_RIGHT_WHEEL]) / 4.0;
  // convert it to linear velocity
  wheel_velocity *= (wheel_diameter_ / 2.0);
  ROS_INFO("%f m/s", wheel_velocity);
  tfScalar yaw, pitch, roll;
  tf::Quaternion q(odom_.pose.pose.orientation.x, odom_.pose.pose.orientation.y,
                   odom_.pose.pose.orientation.z, odom_.pose.pose.orientation.w);
  tf::Matrix3x3 ypr(q);
  ypr.getEulerYPR(yaw, pitch, roll);

  // assuming the center of gravity is in the middle of the robot
  double lf = wheel_to_wheel_dist_ / 2.0;
  double lr = lf;
  double beta = atan((lr / (lf + lr)) * tan(steering_angle));

  double x_dot = wheel_velocity * cos(yaw + beta); 
  double y_dot = wheel_velocity * sin(yaw + beta); 
  yaw = (wheel_velocity / lr) * sin(beta);

  odom_.pose.pose.position.x = x_dot;
  odom_.pose.pose.position.y = y_dot;
  odom_.pose.orientation = geometry_msgs.msg.Quaternion(*tf_conversions.transformations.quaternion_from_euler(roll, pitch, yaw))
  ROS_INFO("%f", x_dot);
}