import 'dart:convert';

import '../config/api_config.dart';
import '../models/profile_response.dart';
import 'api_service.dart';

class ProfileService {
  final ApiService _apiService = ApiService();

  Future<ProfileResponse> getProfile() async {
    final response = await _apiService.get(ApiConfig.profileEndpoint);

    final data = jsonDecode(response.body);

    if (response.statusCode == 200 && data is Map<String, dynamic>) {
      return ProfileResponse.fromJson(data);
    }

    throw Exception(
      data is Map && data['detail'] != null
          ? data['detail'].toString()
          : 'Unable to load profile',
    );
  }
}
