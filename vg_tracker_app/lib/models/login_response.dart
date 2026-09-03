class LoginResponse {
  final String status;
  final int userId;
  final String token;

  LoginResponse({
    required this.status,
    required this.userId,
    required this.token,
  });

  factory LoginResponse.fromJson(
    Map<String, dynamic> json,
  ) {
    return LoginResponse(
      status: json['status'] as String,
      userId: json['user_id'] as int,
      token: json['token'] as String,
    );
  }
}